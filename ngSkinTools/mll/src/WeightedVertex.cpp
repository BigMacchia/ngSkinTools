#include <assert.h>
#include <sstream>

#include "WeightedVertex.h"
#include "maya.h"
#include "IPointCloud.h"
#include "GeometryInfo.h"
#include "WeightsModifierEngine.h"
#include "StatusException.h"
#include "utils.h"
#include "SkinLayer.h"

using namespace std;

WeightedVertex::WeightedVertex(GeometryInfo &parent):
	weldedTo(NULL),
	skinWeights(NULL),
	nextSkinWeights(NULL),
	originalSkinWeights(NULL),
	geometry(parent),
	loadsWeights(false),
	selectionWeight(0.0)
{
}

WeightedVertex::~WeightedVertex(void)
{
	
	if (this->skinWeights)
		delete[] this->skinWeights;
	if (this->nextSkinWeights)
		delete[] this->nextSkinWeights;
	if (this->originalSkinWeights)
		delete[] this->originalSkinWeights;

}

void WeightedVertex::addNeighboursBySurface(WeightedVertex &vert) {

	// retreive neighbour indexes
	MObject geom(vert.geometry.skinInputGeomData->object());
	MItMeshVertex itMesh(geom);
	int prevIndex=0;
	MIntArray neighbourIndexes;
	itMesh.setIndex(static_cast<int>(vert.vertNum),prevIndex);
	itMesh.getConnectedVertices(neighbourIndexes);

	// add all verts to calculation and make sure they are present in a calculation
	for (uint i=0,neighbourCount = neighbourIndexes.length();i<neighbourCount;i++) {
		if (vert.geometry.isInvisibleVert(neighbourIndexes[i]))
			continue;

		WeightedVertex * neighbour = vert.geometry.initVertex(neighbourIndexes[i]);
		neighbour->loadsWeights = true;
		this->neighbours[neighbour] = neighbour->vertPosition().distanceTo(this->vertPosition());
	}
}

void WeightedVertex::addNeighboursByVolume(WeightedVertex &vert){

	IPointCloud::IPointIterator *it=this->geometry.engine->volumeAssociationCloud->getNearbyVerts(
		this->geometry.engine->volumeAssociationRadius,vert.vertPosition()*vert.geometry.transform);

	WeightedVertex * neighbour;
	while (it->next()){
		neighbour = reinterpret_cast<WeightedVertex *>(it->getData());
		
		// cloud can return this very same vertex relax data as well, skip that
		// if we already have this neighbour, we should check for that to avoid infinite recursion
		if (neighbour!=this && !containsNeighbour(neighbour)){
			// this is a valid neighbour, add/overwrite it
			neighbour->loadsWeights = true;
			this->neighbours[neighbour] = it->getDistance();


			// TODO: customize weld distance const
			if (it->getDistance()<0.0001) {
				// need to nullify influence of this neighbour
				this->neighbours[neighbour] = 0; 
				this->weldJump(*neighbour);
			}
		}
	}
	
}

void WeightedVertex::addNeighbours(WeightedVertex &vert){
	this->addNeighboursBySurface(vert);
	if (this->geometry.engine->volumeAssociationRadius>0)
		this->addNeighboursByVolume(vert);
}

void WeightedVertex::initNeighbours(){
	// neighbours initialized?
	if (this->weldedTo || !this->neighbours.empty())
		return;
	this->addNeighbours(*this);
}

void WeightedVertex::weldJump(WeightedVertex &vert){
	vert.weldedTo = this;
	this->geometry.engine->makeEditable(&vert);
	this->addNeighbours(vert);

}


void WeightedVertex::swapWeightsBuffers() {

	double * weights = this->nextSkinWeights;
	this->nextSkinWeights = this->skinWeights;
	this->skinWeights = weights;

}

void WeightedVertex::initFreeWeight(double *weights){
	if (!weights)
		weights = this->skinWeights;

	this->totalFreeWeight = 1.0;
	if (geometry.engine->preserveLockedInfluences){
		for (uint i=0;i<this->geometry.inflLocked.size();i++){
			if (this->geometry.inflLocked[i]) {
				this->totalFreeWeight -= weights[i];
			}
		}
	}

	// clarify the result
	if (isCloseToZero(this->totalFreeWeight))
		this->totalFreeWeight = 0;

// this should not happend as we normalize prior to this
/*	if (!isCloseTo(totalWeights,1.0)) {
		// TODO: weights not normalized?
		ostringstream warning;
		warning.setf(ios::scientific);
		warning << "weights not normalized on vertex" << this->vertNum << "sum: " << totalWeights;
		
		
		MGlobal::displayWarning(warning.str().c_str());
		this->totalFreeWeight = 0;
	}
*/
	
}

void WeightedVertex::normalizeWeights(double *weights){
	if (!weights)
		weights = this->skinWeights;

	// get sum of all valid weights
	double totalWeights = 0;
	unsigned int wIndex = 0;
	for (std::vector<bool>::const_iterator i=geometry.influencesMask.begin();i!=geometry.influencesMask.end();i++,wIndex++){
		if (*i)
			totalWeights += weights[wIndex];
	}

	// maybe weights are perfectly normalized already?
	if (isCloseTo(totalWeights,1.0))
		return;
	
	// if for some reason there's no weights on vert at all, we can't normalize 
	// (result is just undefined). leave weights as they are
	if (isCloseToZero(totalWeights))
		return;

	// normalize each weight otherwise
	for (uint i=0;i<this->geometry.numVertWeights();i++){
		if (this->geometry.influencesMask[i])
			weights[i] /= totalWeights;
	}

}
void WeightedVertex::mixInOriginalWeights(double bias){
	if (isCloseTo(bias,1.0))
		return;

	assert(this->originalSkinWeights);

	double * currWeight = this->skinWeights;
	double * originalWeight = this->originalSkinWeights;
	for (size_t i=0,count=this->geometry.numVertWeights();i<count;i++,currWeight++,originalWeight++){
		*currWeight = (*currWeight*bias)+ *originalWeight*(1-bias);
	}
}

void WeightedVertex::applySelectionWeight() {
	if (this->weldedTo)
		this->selectionWeight = this->weldedTo->selectionWeight;

	if (!this->selectionWeight)
		return;
	
	this->mixInOriginalWeights(this->selectionWeight);
}


const MPoint WeightedVertex::vertPosition()const {
	return geometry.vertPositions[static_cast<unsigned int>(vertNum)];
}

void WeightedVertex::initializeSecondaryWeightsBuffers() {
	if (!this->nextSkinWeights){
		size_t weightsArrayLength = this->geometry.numVertWeights();
		this->nextSkinWeights = new double[weightsArrayLength];
		this->originalSkinWeights = new double[weightsArrayLength];
		memcpy(this->originalSkinWeights,this->skinWeights,sizeof(double)*weightsArrayLength);
	}
}

#ifdef _DEBUG
void WeightedVertex::dumpCurrentWeights(const double threshold,double *displayWeights) const{
	if (!displayWeights)
		displayWeights = this->skinWeights;

	bool lineStarted = false;

	for (size_t i=0;i<this->geometry.numVertWeights();i++){
		// don't display small weights, but do display negative weights (might indicate a problem)
		if (displayWeights[i]<threshold && displayWeights[i]>=0)
			continue;

		if (!lineStarted){
			cout << "weights for vertex "<<this->vertNum<<": ";
			lineStarted = true;
		}

		MDagPath geomPath;
		if (this->geometry.getLogicalInfluencePath(this->geometry.inflGlobalToLogical[i],geomPath)){
			cout <<geomPath.partialPathName()<<":"<<displayWeights[i]<<" ";
		}
		else
		if (this->geometry.inflGlobalToLogical[i]==InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX){
			cout <<"[Transparency]:"<<displayWeights[i]<<" ";
		}
		else {
			cout <<"[Unknown influence]:"<<displayWeights[i]<<" ";
		}
	}
	if (lineStarted)
		cout << endl;
}
#endif

double WeightedVertex::getCommonWeightMultiplier(const WeightedVertex &otherVertex){
	if (&this->geometry==&otherVertex.geometry)
		return 1.0;

	double unsharedWeight = 0;
	const double *currWeight = otherVertex.skinWeights;
	for (size_t i=0,count=this->geometry.numVertWeights();i<count;i++,currWeight++){
		if (!this->geometry.influencesMask[i])
			unsharedWeight += *currWeight;
	}

	//DEBUG_COUT_ENDL("unshared weight between "<<*this->getName()<<" and "<<*otherVertex->getName()<<":"<<unsharedWeight);
	return 1.0/(1.0-unsharedWeight);
}

#ifdef _DEBUG
const string & WeightedVertex::getName() {
	if (this->name.length()==0){
		ostringstream strName;
		strName<<"[v "<<this->vertNum<<" in "<<this->geometry.path.partialPathName()<<"]";
		this->name = strName.str();
	}
	return this->name;
}
#endif

