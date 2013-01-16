#include "ngLayerColorDisplayNode.h"
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include "SkinLayerManager.h"
#include "utils.h"
#include "SkinLayerPerformanceTrack.h"
#include <limits>
#include "SkinLayer.h"

const MString ngLayerColorDisplayNode::NODENAME("ngSkinLayerDisplay");
const MTypeId ngLayerColorDisplayNode::NODEID(0x00115A80);


MObject ngLayerColorDisplayNode::attrInMesh;
MObject ngLayerColorDisplayNode::attrOutMesh;
MObject ngLayerColorDisplayNode::attrInEvalTrigger;


unsigned int intPow(unsigned int base,unsigned int degree){
	unsigned int result = 1;
	for (unsigned int i=0;i<degree;i++){
		result *= base;
	}
	return result;
}

class InfluenceColorList: public MColorArray
{
public:
	MColor & getInfluenceColor(unsigned int inflNumber){
		while (this->length()<=inflNumber) {
			// just add a blend of all present colors
			for (unsigned int i=0,count=this->length();i<count;i++){
				MColor &c1 = (*this)[i];
				MColor &c2 = (*this)[(i+1)%count];
				
				this->append((c1+c2)*0.5);
			}
		}
		
		
		return (*this)[inflNumber];
	}

	void addInitialColors(){
		this->clear();
		
		// where all those numbers came from? well guess what,
		// http://www.beantownballoons.com/images/CRYSTAL.gif :]
		this->appendIntColor(0,131,80);
		this->appendIntColor(225,73,90);
		this->appendIntColor(200,68,153);
		this->appendIntColor(142,62,97);
	}

	void appendIntColor(short r,short g,short b){
		append(static_cast<float>(r/255.0),static_cast<float>(g/255.0),static_cast<float>(b/255.0));
	}
};


ngLayerColorDisplayNode::ngLayerColorDisplayNode(void)
{
}

ngLayerColorDisplayNode::~ngLayerColorDisplayNode(void)
{
}

void * ngLayerColorDisplayNode::creator(){
	return new ngLayerColorDisplayNode();
}

MStatus ngLayerColorDisplayNode::initialize(){
	// local attribute initialization
	MStatus status;
	
	
	MFnTypedAttribute meshAttr;
	ngLayerColorDisplayNode::attrInMesh=meshAttr.create( "inMesh", "im", MFnMeshData::kMesh, &status);
	meshAttr.setStorable(false);
	meshAttr.setConnectable(true);
 	status = ngLayerColorDisplayNode::addAttribute( ngLayerColorDisplayNode::attrInMesh );

	// sculpt mesh input
	MFnNumericAttribute numAttr;
	ngLayerColorDisplayNode::attrInEvalTrigger=numAttr.create( "inEvalTrigger", "iet",MFnNumericData::kInt,0,&status);
	numAttr.setStorable(false);
	numAttr.setConnectable(false);
 	status = ngLayerColorDisplayNode::addAttribute( ngLayerColorDisplayNode::attrInEvalTrigger );

	ngLayerColorDisplayNode::attrOutMesh=meshAttr.create( "outMesh", "om", MFnMeshData::kMesh, &status);
	meshAttr.setStorable(false);
	meshAttr.setConnectable(true);
 	status = ngLayerColorDisplayNode::addAttribute( ngLayerColorDisplayNode::attrOutMesh );
 	
	attributeAffects( ngLayerColorDisplayNode::attrInMesh, ngLayerColorDisplayNode::attrOutMesh );
	attributeAffects( ngLayerColorDisplayNode::attrInEvalTrigger, ngLayerColorDisplayNode::attrOutMesh );

	return status;

}

SkinLayer * ngLayerColorDisplayNode::getConnectedSkinLayer(){
	/*MPlug skinClusterPlug(thisMObject(),ngLayerColorDisplayNode::attrInSkinCluster);
	if (!skinClusterPlug.isConnected())
		return NULL;

	MPlugArray connections;
	skinClusterPlug.connectedTo(connections,true,false);
	if (connections.length()==0)
		return NULL;

		*/

	MPlug outMesh(thisMObject(),ngLayerColorDisplayNode::attrOutMesh);
	MPlugArray connections;
	outMesh.connectedTo(connections,false,true);

	// get mesh connection from outMesh
	for (unsigned int i=0;i<connections.length();i++){
		MObject mesh = connections[i].node();
		if (!mesh.hasFn(MFn::kMesh) || !mesh.hasFn(MFn::kDagNode))
			continue; 
		MFnDagNode meshNodeFn(mesh);
		MDagPath path;
		meshNodeFn.getPath(path);

		SkinLayerManager * man = SkinLayerManager::findManager(path);
		if (man)
			return man->currentLayer;
	}

	DEBUG_COUT_ENDL("display node cannot find layer manager");
	return NULL;

}

inline void displayInvalidMask(MColorArray &currColors,const float maskValue,int vertexIndex){
	currColors.set(MColor(maskValue,0,0),vertexIndex);
}

void ngLayerColorDisplayNode::displayCurrentWeights(const SkinLayer &layer){

	const double * weight;
	float floatWeight;
	unsigned int weightIncrement;


	currColors.setLength(layer.getManager().getMeshVertCount());

	layer.getAllCurrWeights(&weight,weightIncrement);
	if (weight==NULL){
		MColor nullColor;

		if (layer.getCurrPaintTarget()==SkinLayer::PAINT_TARGET_MASK) {
			nullColor.set(MColor::kRGB,static_cast<float>(34.0/255.0),static_cast<float>(159.0/255.0),static_cast<float>(211.0/255.0));
		}
		else {
			nullColor.set(MColor::kRGB,0,0,0);
		}
		
		for (int i=0,count=this->currColors.length();i<count;i++){
			this->currColors.set(nullColor,i);
		}
	}
	else if (layer.currTargetIsInfluence()){
		for (int i=0,count=layer.getManager().getMeshVertCount();i<count;i++,weight+=weightIncrement){
			floatWeight = static_cast<float>(*weight);
			this->currColors.set(MColor(floatWeight,floatWeight,floatWeight),i);
		}
	}
	else if (layer.getCurrPaintTarget()==SkinLayer::PAINT_TARGET_MASK){
		MColor black(0,0,0);
		// we excpect data to be in one block for mask weights, will be slightly faster increments
		assert(weightIncrement==1);
		const InfluenceWeightsMap & influenceWeights = layer.getFinalWeightList();
		const double * currInfluenceWeight = influenceWeights.getVertWeights();

		if (currInfluenceWeight==NULL){
			for (int i=0,count=this->currColors.length();i<count;i++,weight++){
				displayInvalidMask(this->currColors,static_cast<float>(*weight),i);
			}
		}
		else
		for (int i=0,count=layer.getManager().getMeshVertCount();i<count;i++,weight++,currInfluenceWeight+=influenceWeights.getNumInfluences()){
			floatWeight = static_cast<float>(*weight);
			if (isCloseToZero(*weight)){
				this->currColors.set(black,i);
				continue;
			}

			bool weightsExist = false;
			assert(influenceWeights.hasTransparencyInfluence);
			// first influence is transparency, so we should skip it when deciding if we've got influences here.
			for (unsigned int j=1;j<influenceWeights.getNumInfluences();j++){
				weightsExist = currInfluenceWeight[j]>0;
				if (weightsExist)
					break;
			}

			if (weightsExist){
				if (*weight>0.99)
					// display mask as white for near 1.0 values
					this->currColors.set(MColor(floatWeight,floatWeight,floatWeight),i);
				else {
					// take a little shade of yellow
					this->currColors.set(MColor(floatWeight,floatWeight,static_cast<float>(0.6*(*weight))),i);
				}
			}
			else {
				// influence weights don't exist for this masked vertex: shade of red
				displayInvalidMask(this->currColors,floatWeight,i);
				this->currColors.set(MColor(floatWeight,0,0),i);
			}
		}
	} // end: mask paint
}

void ngLayerColorDisplayNode::displayLayerWeights(const SkinLayer &layer){
	const InfluenceWeightsMap &weightList = layer.influenceWeightList;


	/*
	 
	 * figure out coloring for the layer; assign weights only for used influences
	 * if color is assigned by weight list physical index, then coloring wont change 
	   while painting - physical indexes don't change
	 * color list should be autogenerated, but first few colors should be chosen by hand
	   distinctive from each other; the rest of the list could then be subdivide-generated
	 */

	InfluenceColorList inflColors;
	inflColors.addInitialColors();
	// color for current influence
	static const MColor currInflColor(1,1,1,1);
	// for unknown verticess
	static const MColor colorUnknownVertice(0,0,0,1);
	//static const double maskFadeOutBias = 0.8;

	// strange way to call max() is caused by windef.h conflict
	static const unsigned int UNDEFINED_CURRENT_INFLUENCE = (std::numeric_limits<int>::max)();

	const unsigned int currInfluencePhysicalIndex = layer.currTargetIsInfluence()
				&&weightList.hasLogicalInfluence(layer.getCurrPaintTarget())?
				weightList.inflLogicalToPhysical.find(layer.getCurrPaintTarget())->second:UNDEFINED_CURRENT_INFLUENCE;
				

	const double *currWeight = weightList.getVertWeights();
	for (unsigned int v=0;v<this->currColors.length();v++){
		if (v>weightList.getNumVerts()){
			this->currColors.set(colorUnknownVertice,v);
		}
		MColor vertColor(0,0,0);

		if (weightList.isInitialized()) {
			// TODO: only include used influences, numInfluences is not a reliable measure

			// assume that weight list we're painting has transparency, and base our indexing on that
			assert(weightList.hasTransparencyInfluence);
			
			// skip transparency
			currWeight++;

			for (unsigned int infl=1,count=static_cast<unsigned int>(weightList.inflPhysicalToLogical.size());infl<count;infl++,currWeight++){

				const MColor &color = infl==currInfluencePhysicalIndex?currInflColor:inflColors.getInfluenceColor(infl-1);
				vertColor += color*static_cast<const float>(*currWeight);
			}

			//if (layer.maskWeightList.isInitialized()) {
			//	vertColor *= static_cast<const float>(1-(1-layer.maskWeightList.getWeight(v))*maskFadeOutBias);
			//}
		}

		this->currColors.set(vertColor,v);
	}
	
}

MStatus ngLayerColorDisplayNode::compute(const MPlug& plug, MDataBlock& dataBlock){
	MStatus status;
	if (plug.attribute() != ngLayerColorDisplayNode::attrOutMesh) {
		// ignore other outputs
		return status;
	}

	_TIMING(SkinLayerPerformanceTrack::instance.timeDisplayNode.start())

	MDataHandle inMeshData = dataBlock.inputValue(ngLayerColorDisplayNode::attrInMesh);
	MDataHandle outMeshData = dataBlock.outputValue(ngLayerColorDisplayNode::attrOutMesh);
	
	
	outMeshData.copy(inMeshData);


	SkinLayer * layer = this->getConnectedSkinLayer();

	if (layer){
		layer->getManager().isDisplayDirty = false;

		MFnMesh meshFn(outMeshData.asMesh());

		this->currColors.setLength(meshFn.numVertices());

		if (layer->currTargetIsInfluence() || !layer->currTargetIsDefined()){
			this->displayLayerWeights(*layer);
		}
		else {
			this->displayCurrentWeights(*layer);
		}

		this->resizeVertexIndexes(this->currColors.length());
		meshFn.setVertexColors(this->currColors,this->vertexIndexes);
	}

	
	dataBlock.setClean(plug);
	_TIMING(SkinLayerPerformanceTrack::instance.timeDisplayNode.stop())

	return status;
}


MPlug ngLayerColorDisplayNode::passThroughToOne(const MPlug &plug) const{
	if (plug.attribute()==ngLayerColorDisplayNode::attrInMesh){
		return MPlug(thisMObject(),ngLayerColorDisplayNode::attrOutMesh);
	}
	

	return MPlug();
}

void ngLayerColorDisplayNode::resizeVertexIndexes(const unsigned int newSize){
	// nothing to do?
	if (this->vertexIndexes.length()==newSize)
		return;

	const unsigned int prevSize = this->vertexIndexes.length();
	this->vertexIndexes.setLength(newSize);
	for (unsigned int i=prevSize;i<newSize;i++){
		this->vertexIndexes[i] = i;
	}
}
