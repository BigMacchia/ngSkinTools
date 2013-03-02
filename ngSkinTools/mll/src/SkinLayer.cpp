#include <assert.h>
#include <algorithm>
#include <sstream>
#include "SkinLayer.h"
#include "defines.h"
#include "SkinLayerManager.h"
#include "utils.h"
#include "StatusException.h"
#include "timer.h"
#include "ProgressWindow.h"
#include "SkinLayerPerformanceTrack.h"
#include "SkinLayerPaintGlobals.h"

using namespace std;

SkinLayer::SkinLayer(const SkinLayerID id,SkinLayerManager &manager):
	parent(NULL),
	manager(manager),
	id(id),
	currPaintTarget(SkinLayer::PAINT_TARGET_UNDEFINED),
	opacity(1.0),
	influenceWeightList(true),
	finalWeightList(false),
	enabled(true)
{
	name = MString("<untitled>");
}

SkinLayer::~SkinLayer(void)
{
}

void SkinLayer::setParent(SkinLayer *parent,int layerPosition){
	_TIMING(SkinLayerPerformanceTrack::instance.resetTimers());
	_TIMING(SkinLayerPerformanceTrack::instance.timerOperationTotal.start());

	if (this->parent){
		std::vector<SkinLayer *> &prevList = this->parent->children;
		std::vector<SkinLayer *>::iterator pos = std::find(prevList.begin(),prevList.end(),this);

		// just make sure we DO are in this parent as of yet - should be this way
		assert(pos!=prevList.end());

		prevList.erase(pos);

		manager.delayedUpdatesState.mixChildWeights(*this->parent);
	}
	this->parent = parent;
	if (parent)  {
		
		if (layerPosition==LAYER_POSITION_LAST || layerPosition==LAYER_POSITION_UNDEFINED || static_cast<size_t>(layerPosition)>=parent->children.size()){
			// add layer last
			this->parent->children.push_back(this);
		}
		else {
			// insert layer into
			vector<SkinLayer *>::iterator it= this->parent->children.begin();
			if (layerPosition>0)
				it += layerPosition;

			this->parent->children.insert(it,this);
		}

		this->influenceWeightList.setParentMap(&this->parent->finalWeightList);
		this->finalWeightList.setParentMap(&this->parent->finalWeightList);
		
		manager.delayedUpdatesState.mixChildWeights(*this->parent);
	}

	// when unparenting happens and this layer is still a curent layer in it's manager,
	// set current layer to none.
	if (!parent){
		if (this->manager.currentLayer==this)
			this->manager.setCurrentLayer(NULL);
	}

	_TIMING(SkinLayerPerformanceTrack::instance.timerOperationTotal.stop());
	_TIMING(SkinLayerPerformanceTrack::instance.printOut("Set Parent"));
}

void SkinLayer::setPaintValue(const PaintMode paintMode, const double intensity,const double opacity,const unsigned int verticeFrom,const unsigned int verticeTo,WeightsChange *oldWeights){

	// not in a condition to perform this paint procedure?
	if (!this->currTargetIsDefined() && paintMode!=pmSmooth)
		return;

	// if later set to true, whole mesh needs update
	bool updateWholeMesh=false;

	_TIMING(SkinLayerPerformanceTrack::instance.timerSetWeight.start());

	if (this->currPaintTarget==PAINT_TARGET_MASK){
		if (oldWeights){
			oldWeights->initializeListRange(verticeFrom,verticeTo,this->maskWeightList);
		}

		if (!this->maskWeightList.isInitialized()){
			updateWholeMesh=true;
		}

		this->maskWeightList.resize(this->manager.getMeshVertCount(),0.0);

		for (unsigned int verticeID=verticeFrom;verticeID<=verticeTo;verticeID++){
			DEBUG_COUT_ENDL("painting vertice"<<verticeID);

			double neighbourSum = 0;
			
			double currOpacity = this->manager.vertSelectionAvailable?this->manager.selectionWeight.getWeight(verticeID)*opacity:opacity;
			manager.lastStrokeInfo.setWeight(verticeID,intensity*currOpacity);

			if (currOpacity==0)
				continue;

			double & value=*this->maskWeightList.getWeightPtr(verticeID);
			if (paintMode==pmSmooth){
				neighbourSum = this->maskWeightList.neighbourSum(verticeID,this->manager.neighbourInfo);
				SkinLayerPaintGlobals::applySmooth(value,currOpacity,intensity,neighbourSum);
			}
			else {
				SkinLayerPaintGlobals::apply(paintMode,value,currOpacity,intensity);
			}
			value=clamp<double>(value,0.0,1.0);
		}
	}
	

	if (this->currTargetIsInfluence() || !this->currTargetIsDefined()){

		if (this->currTargetIsDefined()){
			this->influenceWeightList.addInfluenceMapping(this->manager.getMeshVertCount(),this->currPaintTarget);
		}

		if (oldWeights){
			oldWeights->initializeMapRange(verticeFrom,verticeTo,this->influenceWeightList);
		}

		const unsigned int inflPhysicalIndex = this->currTargetIsDefined()?this->influenceWeightList.getPhysicalIndex(this->currPaintTarget):0;

		for (unsigned int verticeID=verticeFrom;verticeID<=verticeTo;verticeID++){
			double sumPrevWeights = this->influenceWeightList.vertexWeightSum(verticeID);

			double currOpacity = this->manager.vertSelectionAvailable?(this->manager.selectionWeight.getWeight(verticeID)*opacity):opacity;
			manager.lastStrokeInfo.setWeight(verticeID,intensity*currOpacity);
			if (currOpacity==0)
				continue;

			if (paintMode==pmSmooth){
				// apply smooth to each influence
				for (unsigned int infl=0,count=static_cast<unsigned int>(this->influenceWeightList.inflPhysicalToLogical.size());infl<count;infl++){
					double neighbourSum=this->influenceWeightList.neighbourSum(verticeID,infl,this->manager.neighbourInfo);
					double & value = *this->influenceWeightList.getVertWeights(verticeID,infl);
					SkinLayerPaintGlobals::applySmooth(value,currOpacity,intensity,neighbourSum);
					value=clamp<double>(value,0.0,1.0);
				}
			}
			else {
				// apply to current influence only
				double & value = *this->influenceWeightList.getVertWeights(verticeID,inflPhysicalIndex);
				SkinLayerPaintGlobals::apply(paintMode,value,currOpacity,intensity);
				value=clamp<double>(value,0.0,1.0);
			}

			// TODO: decide if normalize should be used
			// upscale must be used if weights previously were at 1.0
			_TIMING(SkinLayerPerformanceTrack::instance.timerNormalize.start())
			
			// don't skip current influence when normalizing; include transparency into normalization calculations when smoothing
			const bool smoothMode = paintMode==pmSmooth;
			this->influenceWeightList.normalizeWeights(verticeID,smoothMode?InfluenceWeightsMap::UNDEFINED_PHYSICAL_INFLUENCE:inflPhysicalIndex,sumPrevWeights,!smoothMode);
			_TIMING(SkinLayerPerformanceTrack::instance.timerNormalize.stop())
		}
	}
	
	_TIMING(SkinLayerPerformanceTrack::instance.timerSetWeight.stop());

	if (updateWholeMesh){
		this->notifyWeightsChanged();
	}
	else {
		this->notifyWeightsChanged(verticeFrom,verticeTo);
	}
}

void SkinLayer::restoreWeights(WeightsChange &weightsChange,WeightsChange *weightsChange2){
	weightsChange.restore();

	if (weightsChange2){
		weightsChange2->restore();
	}

	if (weightsChange.getRandomAccess()){
		const unsigned int * w= weightsChange.getRandomAccess();
		for (unsigned int i=0;i<weightsChange.getRandomAccessLength();i++,w++){
			// update weights upstream
			this->notifyWeightsChanged(*w,*w);
		}
	}
	else {
		this->notifyWeightsChanged(weightsChange.getFirstVert(),weightsChange.getLastVert());
	}
}

void SkinLayer::getAllCurrWeights(const double ** const weights,unsigned int &increment) const{
	if (this->currPaintTarget==PAINT_TARGET_MASK){
		*weights = maskWeightList.isInitialized()?maskWeightList.getWeights():NULL;
		increment = 1;
		return;
	}

	if (this->currTargetIsInfluence()){
		if (!this->currTargetIsDefined() || !this->influenceWeightList.hasLogicalInfluence(this->currPaintTarget)){
			*weights = NULL;
			return;
		}
		*weights = this->influenceWeightList.getLogicalInfluence(this->currPaintTarget);
		assert(*weights!=NULL);
		increment = static_cast<unsigned int>(this->influenceWeightList.getNumInfluences());
		return;
	}

	*weights = NULL;
	return;
}


void SkinLayer::listInfluences(MStringArray &inflNames, MIntArray &inflIDs,bool activeOnly){
	MStatus status;
	if (this->manager.skinClusterHandle==MObject::kNullObj)
		return;
	MFnSkinCluster skinClusterFn(this->manager.skinClusterHandle,&status);
	if (status!=MStatus::kSuccess)
		return;

	MDagPathArray influences;
	skinClusterFn.influenceObjects(influences);
	for (unsigned int i=0;i<influences.length();i++){
		int inflIndex = skinClusterFn.indexForInfluenceObject(influences[i]);
		if (!activeOnly || this->influenceWeightList.isLogicalInfluenceUsed(inflIndex)){
			inflNames.append(influences[i].partialPathName());
			inflIDs.append(inflIndex);
		}
	}
}

void SkinLayer::setCurrPaintTarget(const int paintTarget){
	this->currPaintTarget = paintTarget;
	this->manager.invalidateDisplay();
}

void SkinLayer::notifyWeightsChanged()
{
	this->notifyWeightsChanged(0,this->manager.getMeshVertCount()-1);
}

void SkinLayer::mixChildWeights(){
	this->mixChildWeights(0,this->manager.getMeshVertCount()-1);
}

void SkinLayer::mixChildWeights(const int firstVert,const int lastVert){
	if (this->children.empty())
		return;

	_TIMING(SkinLayerPerformanceTrack::instance.timerUpdateWeights.start())

	// reset buffer to zero first
	if (this->finalWeightList.isInitialized()) {
		double *w = this->finalWeightList.getVertWeights(firstVert);
		for (size_t i=0,count=(lastVert-firstVert+1)*this->finalWeightList.getNumInfluences();i<count;i++,w++){
			*w = 0;
		}
	}


	// start new weights
	for (vector<SkinLayer *>::iterator i=this->children.begin();i!=children.end();i++){
		SkinLayer &child = **i;
		if (!child.isEnabled())
			continue;

		for (int v=firstVert;v<=lastVert;v++){
			_TIMING(SkinLayerPerformanceTrack::instance.timerMixWeights.start())
			child.mixOwnWeightsTo(v);
			_TIMING(SkinLayerPerformanceTrack::instance.timerMixWeights.stop())
		}
	}

	this->notifyWeightsChanged(firstVert,lastVert);

	_TIMING(SkinLayerPerformanceTrack::instance.timerUpdateWeights.stop())
}

void SkinLayer::notifyWeightsChanged(const int firstVert,const int lastVert){
	if (this->parent){
		this->parent->mixChildWeights(firstVert,lastVert);
	}
	if (this->manager.rootLayer==this){
		DEBUG_COUT_ENDL("writting skin weights "<<firstVert<<" "<<lastVert);
		this->writeWeightsToSkinCluster(firstVert,lastVert,this->finalWeightList);
	}

	_TIMING(SkinLayerPerformanceTrack::instance.timerUpdateDisplay.start());
	this->manager.invalidateDisplay();
	_TIMING(SkinLayerPerformanceTrack::instance.timerUpdateDisplay.stop());
}


void SkinLayer::mixOwnWeightsTo(const int verticeID){

	InfluenceWeightsMap &selfWeights = this->getFinalWeightList();
	InfluenceWeightsMap &parentWeights=*selfWeights.getParentMap();
	
	// nothing to mix in?
	if (selfWeights.getNumVerts()==0)
		return;

	// weights uninitialized yet?
	if (parentWeights.getNumVerts()==0){
		parentWeights.resize(selfWeights.getNumVerts(),selfWeights.getNumInfluences()+1,true);
	}


	double weightSum = selfWeights.vertexWeightSum(verticeID);

	// expect no more than 1.0 here always, allow calculation to be slightly off
	assert(weightSum<=(1.0+SMALL_NUMBER_LAMBDA) );

	// find out current vertex opacity
	double vertexOpacity = this->getOpacity();
	if (maskWeightList.isInitialized())
		vertexOpacity *= maskWeightList.getWeight(verticeID);


	double realTransparency = 1.0-weightSum*vertexOpacity;

	// reduce previous weights to amount of transparency if own weights prior to adding own weights
	
	// for the following bit to work, parentWeights should not be with a transparency channel
	assert(!parentWeights.hasTransparencyInfluence);
	
	double * targetWeightList = parentWeights.getVertWeights(verticeID);
	for (unsigned int i=0;i<parentWeights.getNumInfluences();i++,targetWeightList++){
		*targetWeightList *= realTransparency;
	}

	// add own weights now
	_TIMING(SkinLayerPerformanceTrack::instance.timerAddOwnWeights.start());
	selfWeights.initializeParentMapping();
	_TIMING(SkinLayerPerformanceTrack::instance.timerAddOwnWeights.stop())
	double * selfWeightList = selfWeights.getVertWeights(verticeID);
	if (selfWeights.hasTransparencyInfluence)
		selfWeightList++;
	for (unsigned int i=(selfWeights.hasTransparencyInfluence?1:0);i<selfWeights.inflPhysicalToLogical.size();i++,selfWeightList++){
		*selfWeights.getParentInfluence(i,verticeID) += *selfWeightList*vertexOpacity;
	}



	// TODO: this was a questionable normalize
	//_TIMING(SkinLayerPerformanceTrack::instance.timerNormalize.start());
	//parentWeights.normalizeWeights(verticeID,NO_PAINT_TARGET,true);
	//_TIMING(SkinLayerPerformanceTrack::instance.timerNormalize.stop())

	// replacing normalize with a requirement that this sum SHOULD always be below 1.0, to keep previous code integrity
	// parent weights should always remain roughly around 1.0, if childs have their weight lists normalized and added up correctly
	assert(parentWeights.vertexWeightSum(verticeID)<(1.0+SMALL_NUMBER_LAMBDA));
}




// cached attributes of SkinClusterNode.weightList.weights
MObject attrWeightList;
MObject attrWeights;

void SkinLayer::writeWeightsToSkinCluster(const int firstVert,const int lastVert, const InfluenceWeightsMap &weights){
	assert(!weights.hasTransparencyInfluence);

	if (lastVert!=firstVert){
		// large amount of weights, write using FnSkinCluster

		// vertex indices
		MFnSingleIndexedComponent verticeSelection;
		MFnSkinCluster skinFn(this->manager.skinClusterHandle);
		MObject vertSelectionObject = verticeSelection.create(MFn::kMeshVertComponent);
		for (int i=firstVert;i<=lastVert;i++){
			verticeSelection.addElement(i);
		}

		// influence indices
		MIntArray inflLogicalIndices;
		skinFn.findPlug("matrix").getExistingArrayAttributeIndices(inflLogicalIndices);
		MIntArray influenceIndices;
		influenceIndices.setLength(inflLogicalIndices.length());
		for (unsigned int i=0;i<inflLogicalIndices.length();i++)
			influenceIndices[i] = i;

		// weight values
		MDoubleArray values;
		values.setLength((lastVert-firstVert+1)*inflLogicalIndices.length());
		for (unsigned int infl=0,totalInfluenes=inflLogicalIndices.length();infl!=totalInfluenes;infl++){
			if (weights.hasLogicalInfluence(inflLogicalIndices[infl])){
				const double * vertWeights = weights.getLogicalInfluence(inflLogicalIndices[infl],firstVert);
				for (int vert=0,numVerts=lastVert-firstVert+1;vert!=numVerts;vert++,vertWeights +=weights.getNumInfluences()){
					values[vert*totalInfluenes+infl] = *vertWeights;
				}
			}
			else {
				// no such influence in weights? just write zeros into it
				for (int vert=0,numVerts=lastVert-firstVert+1;vert!=numVerts;vert++){
					values[vert*totalInfluenes+infl] = 0.0;
				}
			}
		}

		MDagPath path;
		MFnDagNode(this->manager.meshHandle).getPath(path);

		_TIMING(SkinLayerPerformanceTrack::instance.timeAttrWrites.start());
		skinFn.setWeights(path,vertSelectionObject,influenceIndices,values,false,NULL);
		
		_TIMING(SkinLayerPerformanceTrack::instance.timeAttrWrites.stop());

	}
	else  {
		// small amount of weights, write using MPlug

		// look up attribute definitions
		if (attrWeights.isNull()){
			MFnDependencyNode skinClusterNodeFn(this->manager.skinClusterHandle);
			attrWeightList = skinClusterNodeFn.findPlug("weightList").attribute();
			if (!Utils::findChildAttr(attrWeightList,"weights",attrWeights)){
				DEBUG_COUT_ENDL("failed getting weights attribute for skin cluster");
				return;
			}
		}

		//if (weightPlug.isNull()){
		//	weightPlug = MPlug(this->manager.skinClusterHandle.object(),attrWeights);
		//}
		MPlug weightListPlug(this->manager.skinClusterHandle,attrWeightList);
		
		MPlug currVertPlug;
		MStatus status;

		const int vertNum=firstVert;
		const double * currWeight = weights.getVertWeights(vertNum);


		// seek vertices by physical indexes, fall back to logical seeking on error
		currVertPlug = weightListPlug.elementByPhysicalIndex(vertNum,&status).child(attrWeights);
		if (!status){
			weightListPlug.selectAncestorLogicalIndex(vertNum,attrWeightList);
			currVertPlug = weightListPlug.child(attrWeights);
		}
		
		// set old skin cluster weights to zero
		_TIMING(SkinLayerPerformanceTrack::instance.timeAttrWrites.start());
		for (int w=0,count=currVertPlug.numElements();w!=count;w++){
			currVertPlug.elementByPhysicalIndex(w).setDouble(0.0);
		}
		
		// seek influences by logical index
		for (unsigned int i=0;i<weights.getNumInfluences();i++,currWeight++){
			if (*currWeight>0.0){
				currVertPlug.elementByLogicalIndex(weights.inflPhysicalToLogical[i]).setDouble(*currWeight);
			}
		}
		_TIMING(SkinLayerPerformanceTrack::instance.timeAttrWrites.stop());
	}

	return;
}



void SkinLayer::setOpacity(const double opacity){
	this->opacity = opacity;
	this->manager.delayedUpdatesState.notifyWeightsChanged(*this);
}

void SkinLayer::setEnabled(const bool enabled){
	DEBUG_COUT_ENDL("setting layer "<<this->getID()<<" enabled: "<<enabled);
	this->enabled = enabled;
	this->manager.delayedUpdatesState.notifyWeightsChanged(*this);
}

void SkinLayer::setName(const MString &name){
	this->name = name;
}

void SkinLayer::initializeFromSkinCluster(){

	MFnSkinCluster skinCluster(manager.skinClusterHandle);
	// create component list of all used (currently initialized) vertices
	MFnSingleIndexedComponent verticeSelection;
	MObject components = verticeSelection.create(MFn::kMeshVertComponent);
	for (unsigned int i=0;i<manager.getMeshVertCount();i++)
		verticeSelection.addElement(i);

	// get mesh path
	MDagPath meshPath;
	MFnDagNode (manager.meshHandle).getPath(meshPath);

	// get influence paths
	MDagPathArray influences;
	skinCluster.influenceObjects(influences);

	// get weights from skin cluster
	unsigned int inflCount;
	MDoubleArray weights;
	skinCluster.getWeights(meshPath,components,weights,inflCount);

	// load weights into layer now
	this->influenceWeightList.resize(this->manager.getMeshVertCount(),inflCount,false);
	for (unsigned int infl=0;infl<inflCount;infl++){
		this->influenceWeightList.addInfluenceMapping(skinCluster.indexForInfluenceObject(influences[infl]));
	}

	// at this point, physical skin cluster indexing==(physical weight indexing-1), so it's safe
	// to just map physical==physical, right?..
	double * destinationWeight = influenceWeightList.getVertWeights();
	for (unsigned int vtx=0;vtx<influenceWeightList.getNumVerts();vtx++){
		
		// set transparency
		assert(influenceWeightList.hasTransparencyInfluence);
		*destinationWeight = 0.0;
		destinationWeight++;

		for (unsigned int infl=1;infl<influenceWeightList.getNumInfluences();infl++,destinationWeight++){
			*destinationWeight = weights[inflCount*vtx+(infl-1)];
		}
		
		// announce warning if vertices were not normalized in skin cluster
		if (!isCloseTo(influenceWeightList.vertexWeightSum(vtx),1.0)){
			std::ostringstream buff;
			buff << "weights on vertex " << vtx << " are not not normalized";
			MGlobal::displayWarning(buff.str().c_str());
		}

		// perform normalize anyway - because we can. could be a slight overkill, but operation takes 
		// place once per initialization anyway
		influenceWeightList.normalizeWeights(vtx,InfluenceWeightsMap::UNDEFINED_PHYSICAL_INFLUENCE,1.0,true);
	}

	this->manager.delayedUpdatesState.notifyWeightsChanged(*this);
}

void SkinLayer::mirrorWeights(const bool toNegativeSide,const double mirrorWidth,const bool mirrorWeights, const bool mirrorMask, WeightsChange * const prevInfluenceWeights,WeightsChange * const prevMaskWeights){
	InfluenceWeightsMap oldWeights(true);
	oldWeights.copyFromRange(this->influenceWeightList,0,this->influenceWeightList.getNumVerts()-1);


	// should mask be mirrored as well?
	const bool maskExists = this->maskWeightList.getSize()==this->influenceWeightList.getNumVerts() &&
		this->influenceWeightList.isInitialized();

	SkinLayerWeightList oldMask;
	if (maskExists){
		oldMask.copyFromRange(this->maskWeightList,0,this->maskWeightList.getSize()-1);
	}

	this->initializeWeightsChanges(prevInfluenceWeights,prevMaskWeights);

	for (unsigned int i=0;i<this->influenceWeightList.getNumVerts();i++){
		// mirror left to right vertices
		const VertexTransferInfo & vti = this->manager.mirrorData.vertexTransfer.getVertexTransferInfo(i);
		double blendWeight = 0.0;
		if (mirrorWidth==0){
			blendWeight = vti.distanceToCenter<0 ? 1 :0;
		}
		else {
			blendWeight = clamp<double>(0.5 + vti.distanceToCenter*2/mirrorWidth,0,1.0);
		}
			

		if (toNegativeSide)
			blendWeight = 1.0-blendWeight;

		if (isCloseToZero(blendWeight)){
			continue;
		}

		if (mirrorWeights)
			this->influenceWeightList.transferWeights(oldWeights, i,this->manager.mirrorData,blendWeight);

		if (mirrorMask && maskExists)
			this->maskWeightList.transferWeights(oldMask,i,vti,blendWeight);

	}

	this->manager.delayedUpdatesState.notifyWeightsChanged(*this);
}

void SkinLayer::transparencyToMask(WeightsChange *const prevMaskWeights){
	this->initializeWeightsChanges( NULL,prevMaskWeights);

	this->maskWeightList.resize(manager.getMeshVertCount());
	for (unsigned int i=0,count=manager.getMeshVertCount();i<count;i++){
		this->maskWeightList.setWeight(i,this->influenceWeightList.vertexWeightSum(i));
	}
}

void SkinLayer::floodToTransparency(WeightsChange *const prevInfluenceWeights){
	this->initializeWeightsChanges(prevInfluenceWeights,NULL);

	this->influenceWeightList.floodToTransparency(this->getManager().neighbourInfo);

}

void SkinLayer::maskToTransparency(WeightsChange *const prevInfluenceWeights, WeightsChange *const prevMaskWeights){
	if (!this->maskWeightList.isInitialized())
		return;

	
	this->initializeWeightsChanges(prevInfluenceWeights,prevMaskWeights);

	// scale each weight by mask value
	// don't use any smart pointer math, will make code more readable and
	// wont' influence performance by a lot
	for (unsigned int v=0,count=manager.getMeshVertCount();v<count;v++){
		const double maskValue = this->maskWeightList.getWeight(v);
		for (unsigned int infl=0;infl<this->influenceWeightList.getNumInfluences();infl++){
			*this->influenceWeightList.getVertWeights(v,infl) *= maskValue;
		}
		this->influenceWeightList.recalcTransparency(v);
	}

	// delete mask
	this->maskWeightList.release();


}


void SkinLayer::initializeWeightsChanges(WeightsChange *const prevInfluenceWeights, WeightsChange *const prevMaskWeights){
	if (prevInfluenceWeights!=NULL) {
		prevInfluenceWeights->initializeMapRange(0,manager.getMeshVertCount()-1,this->influenceWeightList);
	}
	if (prevMaskWeights!=NULL) {
		prevMaskWeights->initializeListRange(0,manager.getMeshVertCount()-1,this->maskWeightList);
	}
}
