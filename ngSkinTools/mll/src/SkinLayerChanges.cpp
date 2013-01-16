#include <boost/format.hpp>
#include "SkinLayerChanges.h"
#include "defines.h"
#include "ngSkinLayerCmd.h"
#include "ngSkinLayerDataNode.h"
#include "StatusException.h"
#include "SkinLayer.h"
#include "SkinLayerPaintGlobals.h"

namespace SkinLayerChanges {


void AddLayer::execute(){
	layer = manager->createLayer();

	DEBUG_COUT_ENDL("add layer force empty: "<<forceEmpty);
	
	// first layer is initialized from skin cluster
	if (!forceEmpty && manager->rootLayer->children.size()==0){
		layer->initializeFromSkinCluster();
		manager->findDefaultCurrentLayer();
	}

	layer->setName(this->newName);

	redo();	
}
void AddLayer::redo(){
	layer->setParent(this->parent,SkinLayer::LAYER_POSITION_LAST);
}
void AddLayer::undo(){
	layer->setParent(NULL,SkinLayer::LAYER_POSITION_UNDEFINED);
}

void RemoveLayer::execute(){
	this->prevParent = layer->getParent();
	if (this->prevParent)
		this->layerPosition = layer->getIndex();
	redo();	
}
void RemoveLayer::redo(){
	layer->setParent(NULL,this->layerPosition);
	manager->findDefaultCurrentLayer();
}
void RemoveLayer::undo(){
	layer->setParent(this->prevParent,this->layerPosition);
	manager->findDefaultCurrentLayer();
}

void ChangeCurrentLayer::execute(){
	this->prevLayer = manager->currentLayer;
	redo();	
}
void ChangeCurrentLayer::redo(){
	manager->setCurrentLayer(this->layer);
}
void ChangeCurrentLayer::undo(){
	manager->setCurrentLayer(this->prevLayer);
}

void ChangeCurrentInfluence::execute(){
	this->layer = manager->currentLayer;
	if (!layer)
		return;
	this->prevInfluence = manager->currentLayer->getCurrPaintTarget();
	redo();	
}

void ChangeCurrentInfluence::redo(){
	if (!layer)
		return;


	this->layer->setCurrPaintTarget(this->influence);
}
void ChangeCurrentInfluence::undo(){
	if (!layer)
		return;
	this->layer->setCurrPaintTarget(this->prevInfluence);
}

void ChangeLayerIndex::execute(){
	if (layer.getParent()!=NULL){
		this->prevIndex = layer.getIndex();
		this->newIndex = clamp<unsigned int>(this->newIndex,0,static_cast<unsigned int>(this->layer.getParent()->children.size())-1);
	}
	redo();
}
void ChangeLayerIndex::undo(){
	layer.setParent(layer.getParent(),this->prevIndex);
}
void ChangeLayerIndex::redo(){
	layer.setParent(layer.getParent(),this->newIndex);
}


void ChangeLayerName::execute(){
	this->oldName = layer->getName();
	redo();
}
void ChangeLayerName::undo(){
	layer->setName(oldName);
}
void ChangeLayerName::redo(){
	layer->setName(newName);
}

void ChangeOpacity::execute(){
	this->oldOpacity = layer->getOpacity();
	redo();
}
void ChangeOpacity::undo(){
	layer->setOpacity(this->oldOpacity);
}
void ChangeOpacity::redo(){
	layer->setOpacity(this->newOpacity);
}

void ChangeEnabled::execute() {
	this->oldEnabled = layer->isEnabled();
	redo();
}

void ChangeEnabled::redo(){
	layer->setEnabled(this->newEnabled);
}

void ChangeEnabled::undo(){
	layer->setEnabled(this->oldEnabled);
}



void AddDisplay::execute(){
	redo();
}
void AddDisplay::redo(){
	this->manager->insertDisplayNode(this->modifier);
}
void AddDisplay::undo(){
	this->modifier.undoIt();
}

void RemoveDisplay::execute(){
	redo();
}
void RemoveDisplay::redo(){
	SkinLayerManager::deleteDisplayNodes(this->modifier);
}
void RemoveDisplay::undo(){
	this->modifier.undoIt();
}

void AttachData::execute(){
	redo();
}
void AttachData::redo(){
	MObject attachPoint = SkinLayerManager::findManagerAttachPoint(meshPath);
	if (attachPoint==MObject::kNullObj)
		return;

	MObject dataNode = this->modifier.createNode(ngSkinLayerDataNode::NODEID);
	this->modifier.renameNode(dataNode,MString("ngSkinToolsData_")+MFnDependencyNode(attachPoint).name());
	this->modifier.connect(attachPoint,MPxNode::message,dataNode,ngSkinLayerDataNode::attrSkinCluster);
	this->modifier.doIt();
}
void AttachData::undo(){
	this->modifier.undoIt();
}

void MirrorLayer::execute(){
	if (this->guessMirrorSide){
		this->positiveToNegative = manager->lastStrokeInfo.getLastSide()>0;
	}

	redo();
}
void MirrorLayer::redo(){
	if (!layer.getManager().isMirrorCacheValid()){
		layer.getManager().mirrorData.reset();
		throwStatusException("failed to mirror (mirror data is not available)",MStatus::kInvalidParameter);
	}

	layer.mirrorWeights(this->positiveToNegative,this->mirrorWidth,this->mirrorWeights,this->mirrorMask, &this->prevInfluenceWeights,&this->prevMaskWeights);
}
void MirrorLayer::undo(){
	layer.restoreWeights(prevMaskWeights,&prevInfluenceWeights);
} 


void FloodWeights::execute(){
	this->intensity = SkinLayerPaintGlobals::brushIntensity;
	this->paintMode = SkinLayerPaintGlobals::currentPaintMode;
	this->redo();
}

void FloodWeights::redo(){
	manager->initSelectionWeight();
	manager->startPaintStroke();
	this->layer.setPaintValue(this->paintMode,this->intensity,1.0,0,manager->getMeshVertCount()-1,&this->prevWeights);
	manager->endPaintStroke();
}

void FloodWeights::undo(){
	this->layer.restoreWeights(this->prevWeights);
}


void TransparencyToMask::execute(){
	this->redo();
}

void TransparencyToMask::redo(){
	this->layer.transparencyToMask(&this->prevMaskWeights);
	this->layer.floodToTransparency(&this->prevInfluenceWeights);
	layer.getManager().delayedUpdatesState.notifyWeightsChanged(layer);
}


void TransparencyToMask::undo(){
	this->layer.restoreWeights(this->prevMaskWeights,&this->prevInfluenceWeights);
}

void MaskToTransparency::execute(){
	this->redo();
}

void MaskToTransparency::redo(){
	this->layer.maskToTransparency(&this->prevInfluenceWeights,&this->prevMaskWeights);
	layer.getManager().delayedUpdatesState.notifyWeightsChanged(layer);
}


void MaskToTransparency::undo(){
	this->layer.restoreWeights(this->prevMaskWeights,&this->prevInfluenceWeights);
}


void SetLayerWeights::execute(){
	this->manager = &this->layer.getManager();
	assert(target!=SkinLayer::PAINT_TARGET_UNDEFINED);
	assert(this->manager!=NULL);

	if (this->newWeights.length()==0 && getTarget()!=SkinLayer::PAINT_TARGET_MASK){
		throwStatusException("Cannot unset weights for an influence", MStatus::kInvalidParameter);
	}

	if (this->newWeights.length()!=0 && this->newWeights.length()!=this->manager->getMeshVertCount()){
		throwStatusException(boost::format("Invalid vertex count, should be %1%") % this->manager->getMeshVertCount(), MStatus::kInvalidParameter);
	}

	if (getTarget()==SkinLayer::PAINT_TARGET_MASK){
		layer.initializeWeightsChanges(NULL,&this->prevWeights);
	}
	else {
		layer.initializeWeightsChanges(&this->prevWeights,NULL);
	}

	this->redo();
}


inline bool skinClusterHasLogicalInfluence(MObject &skinClusterHandle, const int logicalIndex){
	MStatus status;
	MFnSkinCluster clusterFn(skinClusterHandle,&status);
	CHECK_STATUS("Failed to initialize skincluster data while checking logical influence index",status);
	MPlug plug = clusterFn.findPlug("matrix");
	MIntArray indices;
	plug.getExistingArrayAttributeIndices(indices);
	for (unsigned int i=0;i<indices.length();i++){
		if (indices[i]==logicalIndex)
			return true;
	}
	return false;
}

void SetLayerWeights::redo(){
	if (this->getTarget()==SkinLayer::PAINT_TARGET_MASK){
		this->layer.maskWeightList.setWeights(this->newWeights);
	}
	
	if (this->getTarget()>=0){
		if (!skinClusterHasLogicalInfluence(manager->skinClusterHandle, this->getTarget())){
			throwStatusException(boost::format("skin cluster does not have logical influence %1%") % this->getTarget(),MStatus::kInvalidParameter);
		}
		
		
		this->layer.influenceWeightList.setLogicalInfluenceWeights(this->getTarget(),this->newWeights);
	}

	manager->delayedUpdatesState.notifyWeightsChanged(layer);
}

void SetLayerWeights::undo(){
	this->layer.restoreWeights(this->prevWeights);
}



} // end of namespace
