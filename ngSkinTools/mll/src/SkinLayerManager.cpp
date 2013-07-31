#include "SkinLayerManager.h"
#include "defines.h"
#include "utils.h"
#include "ngLayerColorDisplayNode.h"
#include "StatusException.h"
#include "ngSkinLayerDataNode.h"
#include <maya/MMatrix.h>
#include "SkinLayer.h"


bool SkinLayerManager::isPainting = false;
std::vector<MObjectHandle> SkinLayerManager::autoDisplayNodes;

using namespace std;

void DelayedUpdatesStateManager::suspendEnded(){
	if (updateRootWeights){
		DEBUG_COUT_ENDL("updating manager weights");
		manager->rootLayer->mixChildWeights();
	}
}

SkinLayerManager::SkinLayerManager():
		currentLayer(NULL),
		layerIDGenerator(0),
		meshVertCount(0),
		isDisplayDirty(false),
		vertSelectionAvailable(false),
		influenceLimitPerVert(0)
{
	rootLayer = new SkinLayer(0,*this);
	this->delayedUpdatesState.setManager(this);
	MString name = "<root layer>";
	rootLayer->setName(name);
}

SkinLayerManager::~SkinLayerManager(void)
{
	for (std::map<unsigned int,SkinLayer *>::iterator i=this->layers.begin();i!=this->layers.end();i++){
		SkinLayer * layer = i->second;
		if (layer)
			delete layer;
	}
	delete rootLayer;
}

SkinLayer * SkinLayerManager::createLayer(SkinLayerID id){
	layerIDGenerator++;
	if (id>=layerIDGenerator)
		layerIDGenerator = id;
	SkinLayer * newLayer = new SkinLayer(layerIDGenerator,*this);

	newLayer->influenceWeightList.resize(this->getMeshVertCount(),2,false);

	this->layers[newLayer->getID()] = newLayer;
	return newLayer;
}

void SkinLayerManager::listLayers(MStringArray &result)const{
	this->listLayers(result,*this->rootLayer,0);
}

void SkinLayerManager::displayColorOnSelection(const bool colorsEnabled){
	MSelectionList selection;
	MGlobal::getActiveSelectionList(selection);
	MItSelectionList it(selection);
	for (;!it.isDone(); it.next()){
		MObject node;
		it.getDependNode(node);

		// this selection item can't possibly be mesh shape
		if (!node.hasFn(MFn::kDagNode)){
			continue;
		}

		// if transform is selected, extend to shape
		MDagPath path;
		it.getDagPath(path);
		Utils::saferExtendToShape(path);

		// shape is not mesh?
		if (!path.hasFn(MFn::kMesh)){
			continue;
		}

		node = path.node();
		Utils::enableDisplayColors(node,colorsEnabled);
	}
}

void SkinLayerManager::listLayers(MStringArray &result,const SkinLayer &parent,const int currDepthLevel) const{
	// list layers in reverse order
	for (vector<SkinLayer *>::const_reverse_iterator i=parent.children.rbegin();i!=parent.children.rend();i++){
		SkinLayer *child = *i;
		MString tempStr;

		// ID
		tempStr.set(static_cast<double>(child->getID()),0);
		result.append(tempStr);

		// name
		result.append(child->getName());

		// depth level
		tempStr.set(static_cast<double>(currDepthLevel),0);
		result.append(tempStr);

		this->listLayers(result,*child,currDepthLevel+1);
	}
}

SkinLayer * SkinLayerManager::getLayerByID(SkinLayerID id){
	if (id<=0)
		return this->rootLayer;

	std::map<unsigned int,SkinLayer *>::iterator item = this->layers.find(id);
	if (item!=this->layers.end())
		return item->second;

	return NULL;
}

/**
 * this method can be a part of an undo/redo operation as well.
 */
void SkinLayerManager::setCurrentLayer(SkinLayer * const layer){
	this->currentLayer = layer;
	this->invalidateDisplay();
}


void SkinLayerManager::setSkinCluster(MObject &skinCluster,MObject &mesh){
	this->meshHandle = mesh;
	this->skinClusterHandle = skinCluster;

	MFnSkinCluster skinFn(skinCluster);
	MObjectArray inputs;
	skinFn.getInputGeometry(inputs);

	// TODO: this is not very safe, no checking, but as long as skin cluster is validated to be a  mesh deformer,
	// it should be fine
	MObject skinClusterInputGeometry = inputs[0];
	this->meshVertCount = MFnMesh(skinClusterInputGeometry).numVertices();

	this->neighbourInfo.initialize(skinClusterInputGeometry);
}

/**
 * signals to colorDisplayNode that it should evaluate the mesh again.
 */
void SkinLayerManager::invalidateDisplay(){
	// no need to repeat twice
	if (!isPainting)
		return;

	if (isDisplayDirty)
		return;

	MObject node;
	if (!this->detectDisplayNode(node))
		return;


	MPlug plug(node,ngLayerColorDisplayNode::attrInEvalTrigger);
	plug.setInt((plug.asInt()+1)%10);
	isDisplayDirty = true;
}

bool SkinLayerManager::insertDisplayNode(MDGModifier &modifier){
	MObject node;
	if (this->detectDisplayNode(node))
		return true;

	// no skin cluster?
	if (skinClusterHandle.isNull())
		return false;

	// we're connecting directly under inMesh
	MFnDependencyNode meshNode(meshHandle);
	MPlug meshInGeom= meshNode.findPlug("inMesh");
	if (meshInGeom.isNull())
		return false;

	MPlugArray connections;
	meshInGeom.connectedTo(connections,true,false);

	// there will always be only one connection, we requested incomming connections
	if (connections.length()){
		MStatus status;
		
		// disconnect direct connection and insert node inbetween
		status = modifier.disconnect(connections[0],meshInGeom);
		CHECK_STATUS("failed to disconnect",status);
		if (!modifier.doIt())
			return false;

		// insert node into inMesh connection chain
		MObject node = modifier.createNode(ngLayerColorDisplayNode::NODEID,&status);
		CHECK_STATUS("node not created",status);
		status = modifier.doIt();
		CHECK_STATUS("modifier failed",status);

		SkinLayerManager::autoDisplayNodes.push_back(MObjectHandle(node));


		MPlug inMeshPlug(node,ngLayerColorDisplayNode::attrInMesh);
		if (inMeshPlug.isNull())
			return false;
		MPlug outMeshPlug(node,ngLayerColorDisplayNode::attrOutMesh);
		if (outMeshPlug.isNull())
			return false;

		status = modifier.connect(connections[0],inMeshPlug);
		CHECK_STATUS("color display in mesh plug not connected",status);
		status = modifier.connect(outMeshPlug,meshInGeom);
		CHECK_STATUS("color display out mesh plug not connected",status);
		if (!modifier.doIt())
			return false;

		return true;
	}

	return false;
}

void SkinLayerManager::deleteDisplayNodes(MDGModifier &modifier){
	for (std::vector<MObjectHandle>::iterator i=SkinLayerManager::autoDisplayNodes.begin();i!=autoDisplayNodes.end();i++){
		if (i->isAlive()){
			MObject shapeNode;
			MPlug sourcePlug(i->objectRef(),ngLayerColorDisplayNode::attrOutMesh);
			if (Utils::findOutputMesh(sourcePlug,shapeNode)){
				Utils::enableDisplayColors(shapeNode,false);
			}

			modifier.deleteNode(i->objectRef());
			modifier.doIt();
		}
	}

	autoDisplayNodes.clear();
}

bool SkinLayerManager::isMirrorCacheValid(){
	MDagPathArray influences;
	MFnSkinCluster skinFn(this->skinClusterHandle);
	skinFn.influenceObjects(influences);


	if (mirrorData.getInfluencesList().size()!=influences.length())
		return false;

	for (unsigned int i=0;i<influences.length();i++){
		if (influences[i].fullPathName()!=mirrorData.getInfluencesList()[i]->getPath())
			return false;
	}
	return true;

}

void SkinLayerManager::initSkinMirrorData(RuleDescriptionList &ruleList){
	this->populateWTA(mirrorData);

	ruleList.isMirrorMode = true;
	mirrorData.initInfluenceAssociations(ruleList,mirrorData);
	mirrorData.initVertexTransferFrom(this->meshHandle,true,ruleList.getMirrorAxis());
}

void SkinLayerManager::populateWTA(WeightTransferAssociation &wta){
	wta.reset();

	wta.setVertices(this->meshHandle);

	MDagPathArray influences;
	MFnSkinCluster skinFn(this->skinClusterHandle);
	skinFn.influenceObjects(influences);
	
	for (unsigned int i=0;i<influences.length();i++){
		MDagPath inflPath = influences[i];
		MVector inflPosition = MPoint(0,0,0,1)*inflPath.inclusiveMatrix()+
			MFnTransform(inflPath.node()).rotatePivot(MSpace::kObject)
			
			;
		wta.addInfluence(
				skinFn.indexForInfluenceObject(inflPath),
				inflPosition,
				MFnDependencyNode(inflPath.node()).name(),
				inflPath.fullPathName()
			);
	}
	
}

MObject SkinLayerManager::findManagerAttachPoint(MDagPath &path){
	Utils::saferExtendToShape(path);
	if (!path.hasFn(MFn::kMesh))
		return MObject::kNullObj;

	MObject skinCluster = Utils::findSkinCluster(path);
	if (skinCluster!=MObject::kNullObj)
		return skinCluster;

	DEBUG_COUT_ENDL("no skin cluster on "<<path.fullPathName());
	return MObject::kNullObj;
}

ngSkinLayerDataNode * SkinLayerManager::findRelatedDataNode(MObject &attachPoint,MObject *handle){
	
	// initialize default returns to nulls
	if (handle!=NULL)
		*handle = MObject::kNullObj;

	MPlugArray connections;
	MPlug messagePlug(attachPoint,MPxNode::message);
	messagePlug.connectedTo(connections,false,true);
	

	for (unsigned int i=0;i<connections.length();i++){
		MStatus status;
		MFnDependencyNode node(connections[i].node());
		if (node.typeId()!=ngSkinLayerDataNode::NODEID)
			continue;
		
		if (handle!=NULL)
			*handle = connections[i].node();

		ngSkinLayerDataNode * result = static_cast<ngSkinLayerDataNode *>(node.userNode(&status));
		return status?result:NULL;
	}

	return NULL;
}

#ifdef  _DEBUG
TestManagerLocator TestManagerLocator::instance;
#endif

SkinLayerManager * SkinLayerManager::findManager(MDagPath &path){

	#ifdef  _DEBUG
	// get result from test mapping source first
	SkinLayerManager * result = TestManagerLocator::instance.findManager(path);
	if (result)
		return result;
	#endif


	MObject skinCluster = findManagerAttachPoint(path);
	if (skinCluster==MObject::kNullObj)
		return NULL;
	


	ngSkinLayerDataNode * dataNode = findRelatedDataNode(skinCluster,NULL);

	if (dataNode){
		return dataNode->getManager();
	}
	
	return NULL;
}

bool SkinLayerManager::detectDisplayNode(MObject &node){
	MItDependencyGraph it(this->meshHandle,
			MFn::kDependencyNode,
			MItDependencyGraph::kUpstream,
			MItDependencyGraph::kBreadthFirst,
			MItDependencyGraph::kNodeLevel);

	for (;!it.isDone();it.next()){
		MFnDependencyNode nodeFn(it.thisNode());

		if (nodeFn.typeId()==ngLayerColorDisplayNode::NODEID){
			node = it.thisNode();
			return true;
		}
		
	}
	return false;
}

void SkinLayerManager::initSelectionWeight(){
	MSelectionList vertSelection;
#if MAYA_API_VERSION>200800
	MRichSelection richSelection;
	MGlobal::getRichSelection(richSelection);
	richSelection.getSelection(vertSelection);
#else
	MGlobal::getActiveSelectionList(vertSelection);
#endif
	this->selectionWeight.resize(this->getMeshVertCount());

	vertSelectionAvailable = false;
	for (unsigned int i=0;i<selectionWeight.getSize();i++)
		selectionWeight.setWeight(i,0.0);



	MItSelectionList selectionIterator(vertSelection);
	for (;!selectionIterator.isDone();selectionIterator.next()){
		// get current selection path
		MDagPath path;
		MObject compSelection;
		selectionIterator.getDagPath(path,compSelection);
		Utils::saferExtendToShape(path);
		if (path.node()!=this->meshHandle)
			continue;

		MFnComponent componentInfo(compSelection);

		if (selectionIterator.hasComponents() && compSelection.hasFn(MFn::kMeshVertComponent)){
			vertSelectionAvailable = true;
			// read components now, creating vertexData for each of them
			int currentSelectionElement=0;
			for (MItMeshVertex itComponent(path,compSelection);!itComponent.isDone();itComponent.next(),currentSelectionElement++){
				
#if MAYA_API_VERSION>200800
				if (componentInfo.hasWeights()){
					this->selectionWeight.setWeight(itComponent.index(),componentInfo.weight(currentSelectionElement).influence());
					DEBUG_COUT_ENDL("selection weights for "<<itComponent.index()<<": "<<componentInfo.weight(currentSelectionElement).influence());
				}
				else
					this->selectionWeight.setWeight(itComponent.index(),1.0);
#else
				this->selectionWeight.setWeight(itComponent.index(),1.0);
#endif
			}
		}
	}

}


void SkinLayerManager::transferWeights(SkinLayerManager &source){
	DEBUG_COUT_ENDL("transfering skin weights");

	WeightTransferAssociation sourceWTA,destinationWTA;

	source.populateWTA(sourceWTA);
	this->populateWTA(destinationWTA);
	
	destinationWTA.initVertexTransferFrom(source.meshHandle,false);

	RuleDescriptionList ruleList;
	// TODO: provide rule list
	destinationWTA.initInfluenceAssociations(ruleList,sourceWTA);

	std::vector<SkinLayer *> & sourceLayers = source.rootLayer->children;

	for (std::vector<SkinLayer *>::const_iterator i=sourceLayers.begin();i!=sourceLayers.end();i++){
		SkinLayer * layer = this->createLayer();
		const SkinLayer * const sourceLayer = *i;

		layer->setParent(this->rootLayer);
		this->setCurrentLayer(layer);
		layer->setName(sourceLayer->getName());

		layer->influenceWeightList.resize(this->getMeshVertCount(),1,false);
		layer->influenceWeightList.transferWeights(sourceLayer->influenceWeightList,destinationWTA);

		// copy mask if it's present
		if (sourceLayer->maskWeightList.isInitialized()){
			layer->maskWeightList.resize(this->getMeshVertCount());
			layer->maskWeightList.transferWeights(sourceLayer->maskWeightList,destinationWTA);
		}

	}

}

void SkinLayerManager::findDefaultCurrentLayer(){
	if (this->currentLayer!=NULL)
		return;

	if (this->rootLayer->children.size()==0){
		return;
	}

	this->currentLayer = *this->rootLayer->children.begin();

}

void SkinLayerManager::getInfluencePath(const int logicalInfluenceIndex,MDagPath &path) {
	if (this->skinClusterHandle.isNull())
		return;
	
	MFnSkinCluster clusterFn(this->skinClusterHandle);
	MDagPathArray influences;
	clusterFn.influenceObjects(influences);
	for (unsigned int i=0;i<influences.length();i++){
		if (clusterFn.indexForInfluenceObject(influences[i])==logicalInfluenceIndex){
			path = influences[i];
			return;
		}
	}

}


bool SkinLayerManager::addMirrorInfluenceAssociation(const MString &source,const MString &destination){
	MStatus status;
	unsigned int sourceIndex,destinationIndex;
	CHECK_MSTATUS_AND_RETURN(Utils::findInfluenceInSkinCluster(skinClusterHandle,source,sourceIndex),false);
	CHECK_MSTATUS_AND_RETURN(Utils::findInfluenceInSkinCluster(skinClusterHandle,destination,destinationIndex),false);


	this->mirrorManualOverrides[sourceIndex] = destinationIndex;

	return true;
}
bool SkinLayerManager::removeMirrorInfluenceAssociation(const MString &source,const MString &destination){
	MStatus status;
	unsigned int sourceIndex,destinationIndex;
	CHECK_MSTATUS_AND_RETURN(Utils::findInfluenceInSkinCluster(skinClusterHandle,source,sourceIndex),false);
	CHECK_MSTATUS_AND_RETURN(Utils::findInfluenceInSkinCluster(skinClusterHandle,destination,destinationIndex),false);

	// no such index?
	std::map<unsigned int,unsigned int>::iterator item = mirrorManualOverrides.find(sourceIndex);
	if (item==mirrorManualOverrides.end())
		return false;

	// invalid pair given?
	if (item->second!=destinationIndex)
		return false;

	// ok, can erase now
	mirrorManualOverrides.erase(item);
		
	return true;
}

void SkinLayerManager::setInfluenceLimitPerVert(const unsigned int limit) {
	this->influenceLimitPerVert = limit;
	this->delayedUpdatesState.mixChildWeights(*this->rootLayer);
}