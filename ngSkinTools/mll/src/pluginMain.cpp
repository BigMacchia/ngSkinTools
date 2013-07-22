/**
	this module introduces plugin to maya:
	creats plugin,registers/unregisters commands
*/
#include "ngSkinRelaxCmd.h"
#include "ngListInfluencesCmd.h"
#include "ngAssignWeightsCmd.h"
#include "ngSkinLayerCmd.h"
//#include "ngSkinTransferCmd.h"
#include "ngLayerColorDisplayNode.h"
#include "SkinLayerData.h"
#include "ngSkinLayerDataNode.h"
#include "StatusException.h"

#include <maya/MFnPlugin.h>
#include "pluginVersion.h"
#include "geometrymath.h"

#include "SkinLayerPaintContextCmds.h"

MStatus initializePlugin( MObject obj )
{
	MStatus   status;

	DEBUG_COUT_ENDL("loaded debug version of ngSkinTools");

	MFnPlugin plugin( obj, PLUGIN_VENDOR, PLUGIN_VERSION, "Any");
	try {
		CHECK_STATUS("failed to register ngskinrelax",status);

		status = plugin.registerCommand( ngSkinRelax::COMMAND_NAME, ngSkinRelax::creator,ngSkinRelax::syntaxCreator );
		CHECK_STATUS("failed to register ngskinrelax",status);

		status = plugin.registerCommand( ListInfluences::ngListInfluencesCmd::COMMAND_NAME, ListInfluences::ngListInfluencesCmd::creator,ListInfluences::ngListInfluencesCmd::syntaxCreator );
		CHECK_STATUS("failed to register ngListInfluences",status);

		status = plugin.registerCommand( ngAssignWeightsCmd::COMMAND_NAME, ngAssignWeightsCmd::creator,ngAssignWeightsCmd::syntaxCreator );
		CHECK_STATUS("failed to register ngAssignWeights",status);

		status = plugin.registerCommand( ngSkinLayerCmd::COMMAND_NAME, ngSkinLayerCmd::creator,ngSkinLayerCmd::syntaxCreator );
		CHECK_STATUS("failed to register ngSkinLayer",status);

//		status = plugin.registerCommand( ngSkinTransferCmd::COMMAND_NAME, ngSkinTransferCmd::creator,ngSkinTransferCmd::syntaxCreator );
//		CHECK_STATUS("failed to register "<<ngSkinTransferCmd::COMMAND_NAME);

		// layer paint context commands commands
		status = plugin.registerCommand( LayerPaintCmds::SetValueCmd::COMMAND_NAME, LayerPaintCmds::SetValueCmd::creator,LayerPaintCmds::SetValueCmd::syntaxCreator );
		CHECK_STATUS("failed to register command",status);
		status = plugin.registerCommand( LayerPaintCmds::GetValueCmd::COMMAND_NAME, LayerPaintCmds::GetValueCmd::creator,LayerPaintCmds::GetValueCmd::syntaxCreator );
		CHECK_STATUS("failed to register command",status);
		status = plugin.registerCommand( LayerPaintCmds::InitializeCmd::COMMAND_NAME, LayerPaintCmds::InitializeCmd::creator,LayerPaintCmds::InitializeCmd::syntaxCreator );
		CHECK_STATUS("failed to register command",status);
		status = plugin.registerCommand( LayerPaintCmds::FinalizeCmd::COMMAND_NAME, LayerPaintCmds::FinalizeCmd::creator,LayerPaintCmds::FinalizeCmd::syntaxCreator );
		CHECK_STATUS("failed to register command",status);

		// data types
		status = plugin.registerData(SkinLayerData::typeName,SkinLayerData::id,SkinLayerData::creator);
		CHECK_STATUS("failed to register skin layer data type",status);

		// nodes
		status = plugin.registerNode(ngLayerColorDisplayNode::NODENAME,ngLayerColorDisplayNode::NODEID,ngLayerColorDisplayNode::creator,ngLayerColorDisplayNode::initialize,MPxNode::kDependNode);
		CHECK_STATUS("failed to register layer color display node",status);
		status = plugin.registerNode(ngSkinLayerDataNode::NODENAME,ngSkinLayerDataNode::NODEID,ngSkinLayerDataNode::creator,ngSkinLayerDataNode::initialize,MPxNode::kDependNode);
		CHECK_STATUS("failed to register layer color display node",status);

	}
	catch (StatusException e){
		return e.getStatus();
	}

	return status;
}

MStatus uninitializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj );

	try {
		// status is unchecked here, just force deregistering anyway
		status = plugin.deregisterCommand( ngSkinRelax::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<ngSkinRelax::COMMAND_NAME);
		
		status = plugin.deregisterCommand( ListInfluences::ngListInfluencesCmd::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<ListInfluences::ngListInfluencesCmd::COMMAND_NAME);
		
		status = plugin.deregisterCommand( ngAssignWeightsCmd::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<ngAssignWeightsCmd::COMMAND_NAME);
		
		status = plugin.deregisterCommand( ngSkinLayerCmd::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<ngSkinLayerCmd::COMMAND_NAME);
		
//		status = plugin.deregisterCommand( ngSkinTransferCmd::COMMAND_NAME );
//		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<ngSkinTransferCmd::COMMAND_NAME);
		
		status = plugin.deregisterCommand( LayerPaintCmds::GetValueCmd::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<LayerPaintCmds::GetValueCmd::COMMAND_NAME);
		
		status = plugin.deregisterCommand( LayerPaintCmds::SetValueCmd::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<LayerPaintCmds::SetValueCmd::COMMAND_NAME);
		
		status = plugin.deregisterCommand( LayerPaintCmds::InitializeCmd::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<LayerPaintCmds::InitializeCmd::COMMAND_NAME);
		
		status = plugin.deregisterCommand( LayerPaintCmds::FinalizeCmd::COMMAND_NAME );
		DEBUG_REPORT_ERROR_STATUS("failed to deregister "<<LayerPaintCmds::FinalizeCmd::COMMAND_NAME);

		status = plugin.deregisterNode(ngLayerColorDisplayNode::NODEID);
		DEBUG_REPORT_ERROR_STATUS("failed to deregister color display node");

		status = plugin.deregisterNode(ngSkinLayerDataNode::NODEID);
		DEBUG_REPORT_ERROR_STATUS("failed to deregister layer data node");

		status = plugin.deregisterData(SkinLayerData::id);
		DEBUG_REPORT_ERROR_STATUS("failed to deregister skin layer data");


	}
	catch (StatusException e){
		status.perror("error deregistering command");
		return e.getStatus();
	}

	return MStatus::kSuccess;
}
