#include <stdio.h>
#include <sstream>
#include "SkinLayerPaintContextCmds.h"
#include "StatusException.h"
#include "utils.h"
#include "SkinLayerManager.h"
#include "SkinLayerPerformanceTrack.h"

namespace LayerPaintCmds {

const char  GetValueCmd::COMMAND_NAME[]="ngLayerPaintCtxGetValue";
const char  SetValueCmd::COMMAND_NAME[]="ngLayerPaintCtxSetValue";
const char  InitializeCmd::COMMAND_NAME[]="ngLayerPaintCtxInitialize";
const char  FinalizeCmd::COMMAND_NAME[]="ngLayerPaintCtxFinalize";

LayerPaintStrokeInfo currStrokeInfo;


int LayerPaintStrokeInfo::addSurface(MString surface){
	DEBUG_COUT_ENDL("adding surface "<<surface<<" to paint manager list");

	MSelectionList tempSelection;
	tempSelection.add(surface);
	MDagPath selectedShape;
	tempSelection.getDagPath(0,selectedShape);
	Utils::saferExtendToShape(selectedShape);

	SkinLayerManager * layerManager = SkinLayerManager::findManager(selectedShape);
	if (!layerManager){
		DEBUG_COUT_ENDL("no manager on surface"<<selectedShape.fullPathName());
		return NO_MANAGER;
	}

	this->managers.push_back(layerManager);
	
	layerManager->initSelectionWeight();

	return (static_cast<int>(this->managers.size())-1);

}

SkinLayerManager *LayerPaintStrokeInfo::getManager(const int ID) const{
	if (static_cast<size_t>(ID)<this->managers.size())
		return this->managers[ID];
	return NULL;
}

void LayerPaintStrokeInfo::clear(){
	this->managers.clear();
}

MStatus GetValueCmd::doIt( const MArgList& args ){
	this->setResult(0.0);
	return MS::kSuccess;
}

MSyntax GetValueCmd::syntaxCreator(){
	MSyntax result;
	result.addArg(MSyntax::kLong);
	result.addArg(MSyntax::kLong);
	return result;
}


bool SetValueCmd::keyboardOverrideInverse=false;
bool SetValueCmd::keyboardOverrideSmooth=false;

MStatus SetValueCmd::doIt( const MArgList& args ){
	try{
		_TIMING(SkinLayerPerformanceTrack::instance.timerParseArgs.start())
		MStatus status;

		int surfaceID;



		status = args.get(0,surfaceID);
		status = args.get(1,this->vertexID);
		status = args.get(2,this->opacity);

		if (keyboardOverrideSmooth){
			this->intensity = SkinLayerPaintGlobals::brushIntensitySmooth;
			this->mode = pmSmooth;
		}
		else {
			this->intensity = SkinLayerPaintGlobals::brushIntensity;
			this->mode = SkinLayerPaintGlobals::currentPaintMode;

			if (keyboardOverrideInverse){
				if (mode==pmReplace){
					intensity = std::max<double>(0,1.0-intensity);
				}
			}
		}


		SkinLayerManager * layerManager = currStrokeInfo.getManager(surfaceID);
		
		if (!layerManager){
			throwStatusException("no layers for surface", MS::kFailure);
		}

		if (!layerManager->currentLayer){
			throwStatusException("current layer not available", MS::kFailure);
		}
		
		this->layer = layerManager->currentLayer;

		_TIMING(SkinLayerPerformanceTrack::instance.timerParseArgs.stop())

		_TIMING(SkinLayerPerformanceTrack::instance.timeSetValue.start())
		this->redoIt();
		_TIMING(SkinLayerPerformanceTrack::instance.timeSetValue.stop())

	}
	catch (StatusException &e){
		_TIMING(SkinLayerPerformanceTrack::instance.timerParseArgs.stop())
		_TIMING(SkinLayerPerformanceTrack::instance.timeDisplayNode.stop());

		MGlobal::displayError(MString("SetValue encountered an error")+e.getStatus().errorString());
		return e.getStatus();
	}
	return MS::kSuccess;
}

MSyntax SetValueCmd::syntaxCreator(){
	MSyntax result;
	result.addArg(MSyntax::kLong);
	result.addArg(MSyntax::kLong);
	result.addArg(MSyntax::kDouble);
	return result;
}


MStatus InitializeCmd::doIt( const MArgList& args ){
	try {
		SetValueCmd::cacheKeyboardState();

		MStatus status;
		MArgDatabase argData( syntax(), args, &status );
		CHECK_STATUS("initialize arg data",status);

		MString surfacePath;
		status = argData.getCommandArgument(0,surfacePath);
		CHECK_STATUS("failed getting args",status);

		const int id = doIt(surfacePath);

		this->setResult((boost::format("-id %1%") % id).str().c_str());
		return MS::kSuccess;
	}
	catch (StatusException err){
		return err.getStatus();
	}
}

int InitializeCmd::doIt(const MString &shapeName){
	const int id = currStrokeInfo.addSurface(shapeName);
	if (id==LayerPaintStrokeInfo::NO_MANAGER){
		return id;
		//throwStatusException(boost::format("No manager for shape '%1%'") % shapeName,MS::kFailure);
	}

	currStrokeInfo.getManager(id)->startPaintStroke();

#ifdef _SHOW_TIMERS
	SkinLayerPerformanceTrack::instance.resetTimers();
	SkinLayerPerformanceTrack::instance.timerOperationTotal.start();
#endif

	return id;
}

MSyntax InitializeCmd::syntaxCreator(){
	MSyntax result;
	result.addArg(MSyntax::kString);
	return result;
}

MStatus FinalizeCmd::doIt( const MArgList& args ){
#ifdef _SHOW_TIMERS
	SkinLayerPerformanceTrack::instance.timerOperationTotal.stop();
	SkinLayerPerformanceTrack::instance.printOut();
#endif
	
	try {
		MStatus status;
		MArgDatabase argData( syntax(), args, &status );
		CHECK_STATUS("initialize arg data",status);

		int id;
		status = argData.getCommandArgument(0,id);
		CHECK_STATUS("failed getting args",status);

		doIt(id);
		return MS::kSuccess;
	}
	catch (StatusException err){
		return err.getStatus();
	}
}

void FinalizeCmd::doIt(const int id){
	SkinLayerManager * manager = currStrokeInfo.getManager(id);
	if (manager==NULL){
		throwStatusException(boost::format("No manager for shape id '%1%'") % id,MS::kFailure);
	}

	manager->endPaintStroke();
	currStrokeInfo.clear();
}

MSyntax FinalizeCmd::syntaxCreator(){
	MSyntax result;
	result.addArg(MSyntax::kLong);
	return result;
}


}
