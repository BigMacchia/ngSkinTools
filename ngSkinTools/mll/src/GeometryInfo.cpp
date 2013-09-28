#include "GeometryInfo.h"
#include "WeightsModifierEngine.h"
#include "StatusException.h"
#include "WeightedVertex.h"
#include "maya.h"
#include "defines.h"
#include "utils.h"
#include "SkinLayerManager.h"
#include "SkinLayer.h"

#include <maya/MPxDeformerNode.h>
#include <maya/MFnMatrixData.h>

using namespace std;

GeometryInfo::GeometryInfo(WeightsModifierEngine * const engine):
		engine(engine),
		geomFn(NULL),
		skinFn(NULL),
		skinInputGeomData(NULL),
		layer(NULL),
		layerWeightsChange(NULL)
{
}

GeometryInfo::~GeometryInfo(void)
{
	if (this->geomFn)
		delete this->geomFn;
	if (this->skinFn)
		delete this->skinFn;
	if (this->skinInputGeomData)
		delete this->skinInputGeomData;

	VECTOR_FOREACH(WeightedVertex *,verts,v){
		if (*v) {
			delete (*v);
		}
	}

	if (layerWeightsChange)
		delete layerWeightsChange;
}

void GeometryInfo::init()
{
	MStatus status;


	//  determine if there's a skin layer present
	SkinLayerManager * man = SkinLayerManager::findManager(this->path);
	if (man) {
		this->layer = man->currentLayer;
	}



	this->transform = this->path.inclusiveMatrix();

	this->initGeomFn();


	this->skinFn = new MFnSkinCluster(this->skinCluster,&status); CHECK_STATUS("skinCluster FN in geomInfo init()",status);
	this->skinFn->influenceObjects(this->influences,&status);  CHECK_STATUS("skin influences in geomInfo init()",status);

	// calculated logical <-> physical influence indexes
	for (unsigned int i=0;i<influences.length();i++){
		LogicalIndex logicalIndex = this->skinFn->indexForInfluenceObject(influences[i]);
		size_t physicalIndex = i;
		inflPhysicalToLogical[physicalIndex] = logicalIndex;
		inflLogicalToPhysical[logicalIndex] = physicalIndex;
	}


	// init verts array
	this->verts.assign(this->vertPositions.length(),NULL);
}

void GeometryInfo::initGeomFn(){
	MStatus status;
	// alternate way to get positions: read deformer's input
	MPlug inputGeomPlug = MFnDependencyNode(this->skinCluster).findPlug(MPxDeformerNode::input,&status);CHECK_STATUS("find input plug",status);
	inputGeomPlug = inputGeomPlug.elementByPhysicalIndex(0,&status);CHECK_STATUS("first child of geom plug",status);
	inputGeomPlug = inputGeomPlug.child(MPxDeformerNode::inputGeom,&status);CHECK_STATUS("find input geom plug",status);

	this->skinInputGeomData = new MFnMeshData(inputGeomPlug.asMObject());
	this->geomFn = new MFnMesh(this->skinInputGeomData->object(),&status);CHECK_STATUS("MFnMesh creation in geomInfo init()",status);
	status = this->geomFn->getPoints(this->vertPositions); CHECK_STATUS("retreiving points in geomInfo init()",status);
}

WeightedVertex * GeometryInfo::initVertex(const unsigned int index){
	if (this->verts.size()<=index)
		return NULL;

	if (this->verts[index])
		return this->verts[index];


	WeightedVertex * vert = this->engine->createVertexInfoInstance(*this);
	this->verts[index] = vert;

	vert->vertNum = index;

	return vert;
}

void GeometryInfo::calcLockedInfluences(){
	MStatus status;
	this->inflLocked.assign(this->numVertWeights(),false);

	// layers have no locked influences functionality 
	if (this->layer)
		return;
	
	// no preserve flag set by user?
	if (!this->engine->preserveLockedInfluences) {
		return;
	}

	MFnDependencyNode node(this->skinCluster,&status);	CHECK_STATUS("get skin cluster node",status);
	MPlug lockPlug = node.findPlug("lockWeights",&status);	CHECK_STATUS("get lockWeights",status);

	for (size_t i=0;i<this->numVertWeights();i++){

		// associate influences and lock state via LOGICAL index
		// influence list and "lockWeights" arrays might be ordered differently physically
		MPlug element = lockPlug.elementByLogicalIndex(static_cast<unsigned int>(inflGlobalToLogical[i]),&status);	CHECK_STATUS("get plug by logical index",status);


		this->inflLocked[i] = element.asBool();
	}

}

void GeometryInfo::calcUsedInfluences(MDoubleArray &skinWeights,const unsigned int inflCount){
	/*
	for(std::set<int>::const_iterator i=usedInfluences.begin();i!=usedInfluences.end();i++){
		size_t globalIndex = this->engine->getInfluenceGlobalIndex(
				this->influences[static_cast<unsigned int>(inflLogicalToPhysical[*i])]
			);
		
		this->addLogicalToGlobalMapping(*i,globalIndex);
	}

	if (layer){
		this->addLogicalToGlobalMapping(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX,this->engine->getInfluenceGlobalIndex());
	}
	*/

	usedInfluences.clear();
	if (this->engine->useAllInfluences){
		for (std::map<size_t,LogicalIndex>::iterator i=inflPhysicalToLogical.begin();i!=inflPhysicalToLogical.end();i++){
			this->addLogicalToGlobalMapping(i->second,engine->getInfluenceGlobalIndex(this->influences[static_cast<unsigned int>(i->first)]));
		}
		return;
	}

	for (unsigned int i=0;i<inflCount;i++){
		/* in a particular case, when influence is already used by simulation,
		 * flag it as used anyway, so weights are loaded - even if for current skin skin cluster
		 * selected weights aren't weighted to that
		 */
		for (unsigned int globalIndex=0;globalIndex<this->engine->influences.size();globalIndex++){
			if (this->engine->influences[globalIndex]==this->influences[i]){
				this->addLogicalToGlobalMapping(this->inflPhysicalToLogical[i],globalIndex);
				continue;
			}
		}

		if (!layer) {
			// check usage of this influence in all components
			// influence I weight for component C: weights[C*inflCount+I]= weights[componentRow+I]
			// loop iterator represents "influence position in component row
			for (unsigned int componentRow=i;componentRow<skinWeights.length();componentRow+=inflCount){
				if (skinWeights[componentRow]!=0){
					// influence is used.
					this->addLogicalToGlobalMapping(inflPhysicalToLogical[i],engine->getInfluenceGlobalIndex(influences[i]));
					// no need to continue inner loop
					break;
				}
			}
		}
	}

	// get layer used influences. layer stores influences indexed by logical index
	if (layer) {
		InfluenceWeightsMap &layerInflList = layer->influenceWeightList;


		for (std::vector<unsigned int>::iterator i=layerInflList.inflPhysicalToLogical.begin();i!=layerInflList.inflPhysicalToLogical.end();i++){
				if (*i==InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX){
					this->addLogicalToGlobalMapping(*i,engine->getInfluenceGlobalIndex());
				}
				else{
					const unsigned int inflIndex = static_cast<unsigned int>(inflLogicalToPhysical[*i]);
					this->addLogicalToGlobalMapping(*i,engine->getInfluenceGlobalIndex(influences[inflIndex]));
				}
			
		}

	}

}

void GeometryInfo::initSkinWeights(){
	// create component list of all used (currently initialized) vertices
	MFnSingleIndexedComponent verticeSelection;
	MObject components = verticeSelection.create(MFn::kMeshVertComponent);
	VECTOR_FOREACH(WeightedVertex *,verts,i){
		if (*i && (*i)->loadsWeights) {
			verticeSelection.addElement(static_cast<int>((*i)->vertNum));
		}
	}

	// get skin weights
	MStatus status;
	unsigned int inflCount;
	MDoubleArray skinWeights;
	status = this->skinFn->getWeights(this->path,components,skinWeights,inflCount);
	CHECK_STATUS("skin fn get weights in initSkinWeights()",status);

	// calculate a list of really used influences
	calcUsedInfluences(skinWeights,inflCount);



	// initialize vertex memory
	VECTOR_FOREACH(WeightedVertex *,verts,i){
		if (*i && (*i)->loadsWeights) {
		WeightedVertex &vert = **i;
		vert.skinWeights = new double[this->numVertWeights()];
		for (unsigned int i=0;i<this->numVertWeights();i++)
			vert.skinWeights[i] = 0.0;
		}
	}

	// write weights to vertice info
	// it's safe to use this->path here, because components are
	// made of only those vertice indices that exist in skin cluster input
	if (!layer){
		MItMeshVertex itComponents(this->path,components);
		unsigned int currWeight=0; /// points to weight in weights array
		for (;!itComponents.isDone();itComponents.next()){
			WeightedVertex * vert = this->verts[itComponents.index()];

			// at this point, currWeight points at weights[] point where vert's weight
			// for all influences are written. we only write those weights
			// that are currently used (defined by inflUsage)
			// and index in vert->skinWeights is remapped by localToGlobal map
			for (unsigned int i=0;i<inflCount;i++,currWeight++){
				LogicalIndex logical = inflPhysicalToLogical[i];
				//influence not used
				if (usedInfluences.find(logical)==usedInfluences.end())
					continue;

				vert->skinWeights[inflLogicalToGlobal[logical]] =  skinWeights[currWeight];
			}

		}
	}


	if (layer){
		VECTOR_FOREACH(WeightedVertex *,this->verts,i){
			if (!*i)
				continue;

			WeightedVertex * const v = *i;
			if (!v->loadsWeights)
				continue;

			InfluenceWeightsMap &weights = layer->influenceWeightList;
			VECTOR_FOREACH(unsigned int,weights.inflPhysicalToLogical,influence){
				if (usedInfluences.find(*influence)!=usedInfluences.end()){
					v->skinWeights[inflLogicalToGlobal[*influence]] = *weights.getLogicalInfluence(*influence,static_cast<unsigned int>(v->vertNum));
				}
			}

		}
	}


}

void GeometryInfo::finishInfluenceLists(){
	// look for more influences we can/should use
	// continue from where we finished last time: size of influencesMask
	size_t oldSize = this->numVertWeights();
	for (size_t globalIndex=oldSize;globalIndex<this->engine->influences.size();globalIndex++){

		// lets see if we have influence engine->influences[globalIndex]
		for (unsigned int localIndex=0;localIndex<this->influences.length();localIndex++){
			if (!(engine->influences[globalIndex]==this->influences[localIndex]))
				continue;

			// we got a match!
			const LogicalIndex logicalIndex = inflPhysicalToLogical[localIndex];
			this->addLogicalToGlobalMapping(logicalIndex,globalIndex);
		}
	}

	// need to reallocate memory for vertex weights arrays
	size_t newSize = this->numVertWeights();
	if (oldSize<newSize){
		VECTOR_FOREACH(WeightedVertex *,this->verts,v){
			if (!*v || !(*v)->loadsWeights)
				continue;

			// reallocate size of skin weights
			double *w = new double[newSize];
			memcpy(w,(*v)->skinWeights,oldSize*sizeof(double));
			for (size_t i=oldSize;i<newSize;i++)
				w[i] = 0;
			delete [](*v)->skinWeights;
			(*v)->skinWeights = w;
		}
	}



}

void GeometryInfo::writeSkinWeights(){
	/*
	 write skin weights:
	   1. for every vert that is marked as "loadsWeights"
	   3. for every influence that is marked as used in inflUsage[]
	 */

	// set weights into layer rather than directly into skin cluster ?

	DEBUG_COUT_ENDL("writting skin weights on "<<this->path.fullPathName());

	if (layer){
		int numVerts = 0;
		VECTOR_FOREACH(WeightedVertex *,verts,v){
			const WeightedVertex * const curr = *v;
			if (curr && curr->loadsWeights)
				numVerts++;
		}
		if (!numVerts)
			return;

		// create an array of indiced to update for undo data
		unsigned int * vertIndices = new unsigned int[numVerts];
		unsigned int * currVertIndice = vertIndices;
		VECTOR_FOREACH(WeightedVertex *,verts,v){

			WeightedVertex *curr = *v;
			if (curr && curr->loadsWeights){
				*currVertIndice=static_cast<unsigned int>(curr->vertNum);
				currVertIndice++;
			}
		}

		// initialize undo info
		layerWeightsChange = new WeightsChange();
		layerWeightsChange->initializeMapRandomAccess(vertIndices,numVerts,layer->influenceWeightList);

		VECTOR_FOREACH(WeightedVertex *,verts,v){
			WeightedVertex *curr = *v;
			if (!curr || !curr->loadsWeights)
				continue;


			for (unsigned int i=0;i<numVertWeights();i++){
				LogicalIndex logical = inflGlobalToLogical[i];
				const bool inflUsed = usedInfluences.find(logical)!=usedInfluences.end();
				const bool inflPresent = layer->influenceWeightList.hasLogicalInfluence(logical);

				if (inflUsed || inflPresent){
					// layer could receive this logical influence by other means (like, multiple mesh selection)
					layer->influenceWeightList.addInfluenceMapping(layer->getManager().getMeshVertCount(),logical);

					const double newWeight = inflUsed?curr->skinWeights[i]:0.0;
					// assert 0<weight<1.0 with small error
					assert(newWeight>-SMALL_NUMBER_LAMBDA && newWeight<=1.0+SMALL_NUMBER_LAMBDA);

					*layer->influenceWeightList.getLogicalInfluence(logical,static_cast<unsigned int>(curr->vertNum)) = newWeight;
				}
			}
			layer->influenceWeightList.recalcTransparency(static_cast<unsigned int>(curr->vertNum));
			layer->notifyWeightsChanged(static_cast<int>(curr->vertNum),static_cast<int>(curr->vertNum));
		}
		

		return;
	}


	// influence indices: construct array of used influences
	MIntArray influenceIndices;
	for (std::set<LogicalIndex>::const_iterator i=usedInfluences.begin();i!=usedInfluences.end();i++){
		// we require that all used influences reflect some real influence
		assert(inflLogicalToPhysical.find(*i)!=inflLogicalToPhysical.end());

		influenceIndices.append(static_cast<int>(inflLogicalToPhysical[*i]));
	}

	MDoubleArray values;

	MFnSingleIndexedComponent verticeSelection;
	MObject components = verticeSelection.create(MFn::kMeshVertComponent);



	
	VECTOR_FOREACH(WeightedVertex *,verts,v){
		WeightedVertex *curr = *v;
		if (!curr || !curr->loadsWeights)
			continue;

		verticeSelection.addElement(static_cast<unsigned int>(curr->vertNum));
		for (std::set<LogicalIndex>::const_iterator i=usedInfluences.begin();i!=usedInfluences.end();i++){
			values.append(curr->skinWeights[inflLogicalToGlobal[*i]]);
		}
	}


	this->skinFn->setWeights(this->path,components,influenceIndices,values,false,&this->oldSkinWeights);

}

void GeometryInfo::writeOldSkinWeights(){
	if (layer){
		layer->restoreWeights(*layerWeightsChange);
		return;
	}


	MStatus status;
	MFnSingleIndexedComponent verticeSelection;
	MObject components = verticeSelection.create(MFn::kMeshVertComponent);
	VECTOR_FOREACH(WeightedVertex *,verts,i){
		if (*i && (*i)->loadsWeights)
			verticeSelection.addElement(static_cast<int>((*i)->vertNum));
	}
	MIntArray inflIndices;
	for (unsigned int i=0;i<this->influences.length();i++)
		inflIndices.append(i);
	this->skinFn->setWeights(this->path,components,inflIndices,this->oldSkinWeights,false);
	CHECK_STATUS("skinfn set weights in writeOldSkinWeights()",status);

}
