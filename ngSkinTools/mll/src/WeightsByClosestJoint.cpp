#include  <algorithm>

#include "WeightsByClosestJoint.h"
#include "defines.h"
#include "StatusException.h"
#include "maya.h"
#include "utils.h"
#include "ProgressWindow.h"

WeightsByClosestJoint::WeightsByClosestJoint(void)
{
	this->useAllInfluences = true;
	this->useIntersectionRanking = true;
}

WeightsByClosestJoint::~WeightsByClosestJoint(void)
{
}

bool WeightsByClosestJoint::useInfluence(MDagPath &influence){
	if (this->includedInfluences.isEmpty())
		return true;
	return this->includedInfluences.hasItem(influence);
}

void ClosestJointGeometryInfo::init(){

	GeometryInfo::init();
	influenceFinder.setMeshInfo(&this->skinCluster, this->geomFn,this->skinFn);

	WeightsByClosestJoint & engine = *static_cast<WeightsByClosestJoint *>(this->engine); 
	for (unsigned int i=0,count=this->influences.length();i<count;i++){
		if (engine.useInfluence(this->influences[i]))
			influenceFinder.precalcInfluenceShape(this->influences[i],&this->transform);
	}

}

void WeightsByClosestJoint::execute(){
	this->initVertList();
	this->initVertSoftSelection();
	this->initSkinWeights();

	// calc shortest distance and new weights
	ProgressWindow progress("Rigid Weights",static_cast<const int>(this->vertList.size()));
	VECTOR_FOREACH(WeightedVertex *,this->vertList,i){
		static_cast<VertexClosestJointInfo *>(*i)->calcShortestDistance();
		progress.add();
		if (progress.isCanceled())
			return;
	}

	this->applyEffectIntensity();
	this->applySelectionWeights();
	this->finished();
}



void VertexClosestJointInfo::calcShortestDistance(){
	// calc closest influence
	MDagPath closestInfluence;
	MVector pointPos(this->vertPosition());
	static_cast<ClosestJointGeometryInfo *>(&this->geometry)->influenceFinder.findClosestInfluence(pointPos,closestInfluence,
		static_cast<WeightsByClosestJoint * >(this->geometry.engine)->useIntersectionRanking);


	this->initializeSecondaryWeightsBuffers();

	// write weights now
	double *prevWeight = this->skinWeights;
	double *currWeight = this->nextSkinWeights;
	for (size_t i=0,count=this->geometry.numVertWeights();i<count;i++,currWeight++,prevWeight++){
		
		
		MDagPath currPath;
		if (!this->geometry.getLogicalInfluencePath(geometry.inflPhysicalToLogical[i],currPath))
			continue;
		
		if (currPath==closestInfluence){
			// in special case, when influence is locked, but still is the closest one,
			// it gets it's current weight + all free weight
			*currWeight = this->geometry.inflLocked[i]?*prevWeight+this->totalFreeWeight:this->totalFreeWeight;
		}
		else {
			*currWeight = this->geometry.inflLocked[i]?*prevWeight:0.0;
		}
	}

	this->swapWeightsBuffers();


}
