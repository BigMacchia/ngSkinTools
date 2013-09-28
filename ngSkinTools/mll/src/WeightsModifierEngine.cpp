#include <assert.h>
#include <maya/MFnComponent.h>

#include "GeometryInfo.h"
#include "WeightsModifierEngine.h"
#include "WeightedVertex.h"
#include "IPointCloud.h"
#include "utils.h"
#include "ClusteredPointCloud.h"
#include "timer.h"
#include "LimitWeightsUtil.h"

WeightsModifierEngine::WeightsModifierEngine(void):
	volumeAssociationCloud(NULL),
	volumeAssociationRadius(0),
	preserveLockedInfluences(false),
	softSelectionRadius(0.0),
	useAllInfluences(false),
	isExecuteFinished(false),
	useExternalSoftSelection(false)
{
}

WeightsModifierEngine::~WeightsModifierEngine(void)
{
	for (std::vector<GeometryInfo *>::iterator it=this->geometries.begin();it!=this->geometries.end();++it)
		delete(*it);

	if (this->volumeAssociationCloud)
		delete this->volumeAssociationCloud;

}

void WeightsModifierEngine::writeSkinWeights(){
	// was operation canceled?
	if (!this->isExecuteFinished){
		return;
	}

	VECTOR_FOREACH(GeometryInfo *,this->geometries,i){
		(*i)->writeSkinWeights();
	}
}

void WeightsModifierEngine::writeOldSkinWeights(){
	if (!this->isExecuteFinished){
		return;
	}

	VECTOR_FOREACH(GeometryInfo *,this->geometries,i){
		(*i)->writeOldSkinWeights();
	}
}

void WeightsModifierEngine::makeEditable(WeightedVertex *vert){
	// do we have this vertex in editables list already?
	if (isEditable(vert))
		return;
	vert->loadsWeights = true;
	vert->selectionWeight = 1.0;
	vertList.push_back(vert);
	vertListSet.insert(vert);
}

void WeightsModifierEngine::initializeVertCloud(IPointCloud *cloud){

	// calculate and set vert cloud capacity (total amount of points)
	unsigned int totalGeometry = 0;
	VECTOR_FOREACH(GeometryInfo *,this->geometries,geom){
		totalGeometry += (*geom)->vertPositions.length();
	}
	cloud->setCapacity(totalGeometry);


	VECTOR_FOREACH(GeometryInfo *,this->geometries,geom){

		for (unsigned int v=0,count=(*geom)->vertPositions.length();v<count;v++){
			if (!(*geom)->isInvisibleVert(v)) {
				cloud->addPoint((*geom)->initVertex(v),(*geom)->vertPositions[v]*(*geom)->transform);
			}
		}
	}

}


void WeightsModifierEngine::initSkinWeights(){
	VECTOR_FOREACH(GeometryInfo*,geometries,i){
		(*i)->initSkinWeights();
	}
	VECTOR_FOREACH(GeometryInfo*,geometries,i){
		(*i)->finishInfluenceLists();
		(*i)->calcLockedInfluences();
	}
	VECTOR_FOREACH(WeightedVertex*,vertList,i){
		(*i)->normalizeWeights();
		(*i)->initFreeWeight();
	}

}

size_t WeightsModifierEngine::getInfluenceGlobalIndex(const MDagPath &inflPath){
	// add each of the influence to the list, making sure we're not duplicating same influence twice
	for (unsigned int influence=0;influence<influences.size();influence++){
		if (influences[influence]==inflPath)
			return influence;
	}
	this->influences.push_back(MDagPath(inflPath));
	return this->influences.size()-1;
}

size_t WeightsModifierEngine::getInfluenceGlobalIndex(){
	this->influences.push_back(MDagPath());
	return this->influences.size()-1;
}

GeometryInfo * WeightsModifierEngine::findGeometry(MDagPath &path){
	// already initialized?
	VECTOR_FOREACH(GeometryInfo *,this->geometries,i){
		if ((*i)->path==path)
			return *i;
	}
	return NULL;
}


GeometryInfo * WeightsModifierEngine::initGeometry(MDagPath &path){
	
	// is geometry already initialized on this path? if so, just return that instance
	{
		GeometryInfo * geomInfo = findGeometry(path);
		if (geomInfo)
			return geomInfo;
	}

	// find skin cluster on this geometry
	MObject skinCluster = Utils::findSkinCluster(path);
	if (skinCluster.isNull()){
		//MGlobal::displayWarning(path.fullPathName()+" does not have a skin cluster, skipping");
		return NULL;
	}

	GeometryInfo * geomInfo = this->createGeometryInfoInstance();

	geomInfo->path = path;
	geomInfo->skinCluster = skinCluster;
	geomInfo->init();
	this->geometries.push_back(geomInfo);
	return geomInfo;
}

void WeightsModifierEngine::updateSoftSelection(WeightedVertex * vert,const bool inside,const double distanceToWeightRatio){

	vert->initNeighbours();

	// if vert is welded, we don't care about it's selection value or it's children
	if (vert->weldedTo)
		return;

	WeightedNeigbourMap & neighbours = vert->getNeighbours();
	for (WeightedNeigbourMap::iterator n = neighbours.begin();n!=neighbours.end();n++){
		if (n->first->weldedTo)
			continue;

		// new weight:
		// if inside, neighbourWeight = vertWeight+neighbourDistance*distanceToWeightRatio
		// if outside, neighbourWeight = vertWeight-neighbourDistance*distanceToWeightRatio
		// in both cases, 0.0 < neighbourWeight < 1.0
		double newSelWeight = vert->selectionWeight;

		if (inside)
			newSelWeight += n->second*distanceToWeightRatio;
		else
			newSelWeight -= n->second*distanceToWeightRatio;


		// if weights are nearly the same, skip further stuff
		if (isCloseTo(newSelWeight,n->first->selectionWeight)){
			continue;
		}


		// this works for both inside and outside checks (true for smalelr for inside, higher for outside)
		if (inside == (newSelWeight<n->first->selectionWeight)){
			this->makeEditable(n->first);
			n->first->selectionWeight = newSelWeight;
			updateSoftSelection(n->first,inside,distanceToWeightRatio);
		}


	}
}

void WeightsModifierEngine::initVertSoftSelection(){
	if (this->useExternalSoftSelection)
		// we already have a soft selection initialized from maya's selection
		return;

	if (!this->softSelectionRadius)
		// soft selection won't be available anyway
		return;

	double expandBias = 1; // 0: expand selection to inside; 1: expand selection to outside
	double distanceToWeightRatio = 1.0/softSelectionRadius;
	/*
	algorithm:
		1. determine border verts list:
			border verts got a neighbour with selectionWeight 0.0
			border verts get a weight
		2. blur inside:
		   foreach vertex I in border list, process neighbour N:
		     set neighbour N to selection weight (I.weight + distance(IN))
			    only if it's smaller than current N.weight

		3. blur outside:
		    similar as inside; just skip inside verts, and override N.weight if
			it's higher than current N.weight

	*/

	set<WeightedVertex *> softSelectQueue;

	// init border. border vert is a vert that has non-editable verts as it's neighbour
	// initializing neighbours modifies vertList so need to only iterate first few original vertices of the selection
	for (size_t i=0,originalVertsCount=this->vertList.size();i<originalVertsCount;i++){
		WeightedVertex * const vert = this->vertList[i];
		vert->initNeighbours();
		WeightedNeigbourMap & neighbours = vert->getNeighbours();
		for (WeightedNeigbourMap::iterator n = neighbours.begin();n!=neighbours.end();n++){
			if (!isEditable(n->first)){
				// border hit!
				softSelectQueue.insert(vert);
				vert->selectionWeight = expandBias;
				// no need to check neighbours anymore
				break;
			}
		}
	}

	// update inside
	for (set<WeightedVertex *>::iterator it=softSelectQueue.begin();it!=softSelectQueue.end();it++){
		updateSoftSelection(*it,true,distanceToWeightRatio);
	}

	// update outside
	for (set<WeightedVertex *>::iterator it=softSelectQueue.begin();it!=softSelectQueue.end();it++){
		updateSoftSelection(*it,false,distanceToWeightRatio);
	}

	// apply ease-ins and ease-outs (sin curve )
	VECTOR_FOREACH(WeightedVertex *,this->vertList,v){
		const double PI = 3.14;
		const double PI12 = PI/2;
		(*v)->selectionWeight = (sin((*v)->selectionWeight*PI-PI12)+1.0)/2.0;
	}
}




void WeightsModifierEngine::initVertList(){
	MStatus status;

#if MAYA_API_VERSION>200800
	// convert vertSelection with rich selection
	if (this->useExternalSoftSelection) {
		MRichSelection richSelection;
		MGlobal::getRichSelection(richSelection);
		richSelection.getSelection(this->vertSelection);
	}
#endif

	/*
		we support component selections and whole mesh selections;
		in later case, we iterate through all vertices;
		unfortunately, there's a whole in maya's API where iterating through
		component selection,there might be entries where a whole mesh is returned;

		to work around that, we mark each geometry as "wanting to use all components"
		and "already used individual components"

		after iteration through selection, any geometry that "wants all" and didn't "use individual",
		gets it's request satisfied and marks all components as editable
	*/
	std::set<GeometryInfo *> usedIndividualComponents;
	std::set<GeometryInfo *> wantsAllComponents;


	MItSelectionList selectionIterator(this->vertSelection);
	for (;!selectionIterator.isDone();selectionIterator.next()){
		

		// get current selection path
		MDagPath path;
		MObject compSelection;
		selectionIterator.getDagPath(path,compSelection);
		Utils::saferExtendToShape(path);


		// create and initialize geometry info on this skin cluster
		GeometryInfo * geomInfo = this->initGeometry(path);

		// could not init geometry info for this path?
		if (!geomInfo)
			continue;

#if MAYA_API_VERSION>200800
		MFnComponent componentInfo(compSelection);
#endif
		if (!selectionIterator.hasComponents() && path.hasFn(MFn::kMesh)) {
			wantsAllComponents.insert(geomInfo);
		}

		if (selectionIterator.hasComponents() && compSelection.hasFn(MFn::kMeshVertComponent)){
			// mark this geom info that it used individual vertex selection (to block "all components" request)
			usedIndividualComponents.insert(geomInfo);

			// read components now, creating vertexData for each of them
			int currentSelectionElement=0;
			for (MItMeshVertex itComponent(path,compSelection);!itComponent.isDone();itComponent.next(),currentSelectionElement++){
				WeightedVertex *vert = geomInfo->initVertex(itComponent.index());
				if (!vert)
					// failed initializing vertex, probably because such index does not exist in skin cluster input
					continue;

				this->makeEditable(vert);

#if MAYA_API_VERSION>200800
				if (this->useExternalSoftSelection && componentInfo.hasWeights()){
					vert->selectionWeight = componentInfo.weight(currentSelectionElement).influence();
				}
#endif
			}
		}

	}


	// now go through all geometries that want all components, but only satisfly the request
	// if geometry did not have any "individual component" requests
	for (std::set<GeometryInfo *>::iterator i=wantsAllComponents.begin();i!=wantsAllComponents.end();i++){
		
		if (usedIndividualComponents.find(*i)==usedIndividualComponents.end()){
			MFnMesh meshFn((*i)->path);
			for (unsigned int j=0,count=meshFn.numVertices();j<count;j++){
				this->makeEditable((*i)->initVertex(j));
			}
		}
	}
}


void WeightsModifierEngine::initInvisibleVerts(){
	MStatus status;

	// iterate over whole dag
	MItSelectionList selectionIterator(this->invisibleVerts);
	for (;!selectionIterator.isDone();selectionIterator.next()){
		MDagPath path;
		MObject compSelection;
		selectionIterator.getDagPath(path,compSelection);
		if (!selectionIterator.hasComponents() || !compSelection.hasFn(MFn::kMeshVertComponent))
			continue;

		GeometryInfo * geom = findGeometry(path);
		if (!geom)
			continue;

		MIntArray components;
		MItMeshVertex itComponent(path,compSelection);

		for (;!itComponent.isDone();itComponent.next()){
			geom->invisibleVerts.insert(itComponent.index());
		}
	}
}

void WeightsModifierEngine::applySelectionWeights(){
	if (this->softSelectionRadius>0){
		VECTOR_FOREACH(WeightedVertex *,vertList,v){
			(*v)->applySelectionWeight();
		}
	}
}

void WeightsModifierEngine::initVolumeAssociationCloud(){
	if (this->volumeAssociationRadius>0 && this->volumeAssociationCloud==NULL){
		this->volumeAssociationCloud = new ClusteredPointCloud(this->volumeAssociationRadius);
		this->initializeVertCloud(this->volumeAssociationCloud);
	}
}

GeometryInfo * WeightsModifierEngine::createGeometryInfoInstance(){
	return new GeometryInfo(this);
}

WeightedVertex * WeightsModifierEngine::createVertexInfoInstance(GeometryInfo &parent){
	return new WeightedVertex(parent);
}

void WeightsModifierEngine::finished(){
	this->isExecuteFinished = true;
}

bool WeightsModifierEngine::canUndo() const{
	return this->isExecuteFinished;
}

