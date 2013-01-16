#include <maya/MFnTypedAttribute.h>
#include <maya/MFnMessageAttribute.h>
#include "maya.h"
#include "ngSkinLayerDataNode.h"
#include "SkinLayerData.h"
#include "StatusException.h"
#include "SkinLayerManager.h"
#include <maya/MFnPluginData.h>



MTypeId ngSkinLayerDataNode::NODEID(0x00115A82);
char ngSkinLayerDataNode::NODENAME[] = "ngSkinLayerData";
MObject ngSkinLayerDataNode::attrData;
MObject ngSkinLayerDataNode::attrSkinCluster;

ngSkinLayerDataNode::ngSkinLayerDataNode(void)
{
	manager = new SkinLayerManager();
}

ngSkinLayerDataNode::~ngSkinLayerDataNode(void)
{
	delete manager;
}

void ngSkinLayerDataNode::postConstructor(){
	MDataBlock dataBlock = this->forceCache();

	MFnPluginData dataFn;
	MObject skinLayerData = dataFn.create(SkinLayerData::id);
	SkinLayerData * sld = (SkinLayerData *)dataFn.data();
	sld->setValue(this->manager);

	dataBlock.outputValue(attrData).set(skinLayerData);



}

void ngSkinLayerDataNode::copyInternalData( MPxNode* pSrc ){
	DEBUG_COUT_ENDL("fixing internal data after copy");
	this->updateLayerData();
}


/**
 * updates layer data attribute so that it reflects the layer manager of this data node
 */
void ngSkinLayerDataNode::updateLayerData(){
	DEBUG_COUT_ENDL("updating layer data");
	MDataHandle dataHandle= forceCache().inputValue(attrData);
	MStatus status;


	SkinLayerData * data = NULL;
	{	
		MFnPluginData dataPlugFn(dataHandle.data(),&status);
		if (!status){
			DEBUG_COUT_ENDL("could not create data plug fn from plug mobject");
		}
		else{
			data = (SkinLayerData *)dataPlugFn.data(&status);
			if (!status){
				DEBUG_COUT_ENDL("problems getting that plug fn data");
				data = NULL;
			}
		}
	}

	if (data!=NULL){
		DEBUG_COUT_ENDL("reusing data object "<<data->name());
		data->setValue(manager);
	}
	else {
		DEBUG_COUT_ENDL("setting new data object");
		MFnPluginData dataPlugFn;
		MObject dataObj = dataPlugFn.create(SkinLayerData::id);
		SkinLayerData * data = static_cast<SkinLayerData *>(dataPlugFn.data());
		data->setValue(manager);

		forceCache().outputValue(attrData).set(dataObj);
	}
}

MStatus ngSkinLayerDataNode::compute(const MPlug &plug, MDataBlock &dataBlock){
	dataBlock.setClean(plug);
	return MS::kSuccess;
}

MStatus ngSkinLayerDataNode::initialize(){
	MStatus status;
	MFnTypedAttribute layerDataAttrFn;
	ngSkinLayerDataNode::attrData = layerDataAttrFn.create("layerData","ld",SkinLayerData::id);
	layerDataAttrFn.setStorable(true);
	layerDataAttrFn.setConnectable(false);
 	status = ngSkinLayerDataNode::addAttribute( ngSkinLayerDataNode::attrData );
	CHECK_STATUS("problem adding data attribute to node",status);


	MFnMessageAttribute skinClusterAttrFn;
	ngSkinLayerDataNode::attrSkinCluster = skinClusterAttrFn.create("skinCluster","sc");
	skinClusterAttrFn.setConnectable(true);
 	status = ngSkinLayerDataNode::addAttribute( ngSkinLayerDataNode::attrSkinCluster );
	CHECK_STATUS("problem adding skin cluster attribute to node",status);

	return MS::kSuccess;
}

void * ngSkinLayerDataNode::creator(){
	return new ngSkinLayerDataNode();
}

SkinLayerManager * ngSkinLayerDataNode::getManager(){
	if (!manager->skinClusterHandle.isNull())
		return manager;

	// attempting to create manager now

	MStatus status;

	// get connected skin cluster
	MPlug skinClusterPlug(thisMObject(),attrSkinCluster);
	MPlugArray connections;
	skinClusterPlug.connectedTo(connections,true,false,&status);
	CHECK_STATUS("could not get connections from skin cluster",status);
	
	// no skin clusters connected?
	if (connections.length()!=1)
		return NULL;

	MObject skinCluster = connections[0].node();
	if (!skinCluster.hasFn(MFn::kSkinClusterFilter)){
		DEBUG_COUT_ENDL("wrong node connected into skin cluster attribute");
		return NULL;
	}

	// now, get the mesh skin cluster is driving.
	MFnSkinCluster skinFn(skinCluster);
	MObjectArray inGeoms;
	status = skinFn.getOutputGeometry(inGeoms);
	CHECK_STATUS("get input geometry failed",status);

	// no geometries connected for this skin cluster, or connected
	// too many?
	if (inGeoms.length()!=1)
		return NULL;

	MObject meshNode = inGeoms[0];
	// we don't handle non-mesh shapes
	if (!meshNode.hasFn(MFn::kMesh)){
		DEBUG_COUT_ENDL("skin cluster not connected to mesh node");
		return NULL;
	}

	this->manager->setSkinCluster(skinCluster,meshNode);

	this->updateLayerData();

	return this->manager;
}
