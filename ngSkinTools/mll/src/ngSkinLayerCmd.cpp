#include <assert.h>
#include "ngSkinLayerCmd.h"
#include "StatusException.h"
#include "SkinLayerManager.h"
#include "SkinLayerChanges.h"
#include "utils.h"
#include "SkinLayer.h"
#include "SkinLayerPaintGlobals.h"
#include "defines.h"

const char ngSkinLayerCmd::COMMAND_NAME[] = "ngSkinLayer";


using namespace SkinLayerCmd;

ngSkinLayerCmd::ngSkinLayerCmd(void):
	change(NULL)
{
}


ngSkinLayerCmd::~ngSkinLayerCmd(void)
{
	if (change){
		delete change;
	}

}







SkinLayer * ngSkinLayerCmd::getOptionalLayer(){
	if (argData->isFlagSet(FlagNames::LAYERID)){
		return getLayerFromArgId(FlagNames::LAYERID,"no layer under given ID");
	}
	return layerManager->currentLayer;
}


inline const char * axisToString(SkinToolsGlobal::Axis axis){
	if (axis==SkinToolsGlobal::X)
		return "x";
	if (axis==SkinToolsGlobal::Y)
		return "y";
	if (axis==SkinToolsGlobal::Z)
		return "z";
	return "undefined";
}

MStatus ngSkinLayerCmd::handleQuery(){

	// return layer list
	if (argData->isFlagSet(FlagNames::LISTLAYERS)){
		requireManager();

		MStringArray result;
		layerManager->listLayers(result);
		setResult(result);
		return MStatus::kSuccess;
	}

	// list layer subdetail (influences), first number - current influence
	if (argData->isFlagSet(FlagNames::LISTLAYERINFLUENCES)){
		requireManager();

		MStringArray finalResult;

		SkinLayer * layer = getOptionalLayer();

		if (layer){
			MStringArray names;
			MIntArray ids;

			// add active influence as first item
			MString tempStr;
			tempStr.set(static_cast<double>(layer->getCurrPaintTarget()),0);
			finalResult.append(tempStr);
			

			layer->listInfluences(names,ids,argData->isFlagSet(FlagNames::ACTIVEINFLUENCES));
			assert(names.length()==ids.length());
			for (unsigned int i=0;i<names.length();i++){
				tempStr.set(static_cast<double>(ids[i]),0);
				finalResult.append(names[i]);
				finalResult.append(tempStr);
			}
		}
		setResult(finalResult);
		return MStatus::kSuccess;
	}

	
	if (argData->isFlagSet(FlagNames::LAYERDATAATTACH)){
		DEBUG_COUT_ENDL("querying layer data attach");
		setResult(static_cast<bool>(layerManager!=NULL));
		return MS::kSuccess;
	}

	// get current layer
	if ( argData->isFlagSet(FlagNames::CURRLAYER)){
		requireManager();

		if (layerManager->currentLayer){
			setResult(static_cast<int>(layerManager->currentLayer->getID()));
		}
		else
			setResult(SkinLayerManager::UNDEFINED_LAYER_ID);
		
		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::CURRINFLUENCE)){
		const SkinLayer * layer = getOptionalLayer();
		if (layerManager && layer){
			appendToResult(static_cast<int>(layer->getCurrPaintTarget()));

			if (layer->currTargetIsInfluence()){
				MDagPath influencePath;
				layerManager->getInfluencePath(layer->getCurrPaintTarget(),influencePath);
				DEBUG_COUT_ENDL("curr influence path"<<influencePath.fullPathName());
				appendToResult(influencePath.fullPathName());
			}
		}
		else {
			setResult(SkinLayer::PAINT_TARGET_UNDEFINED);
		}
		return MStatus::kSuccess;
	}

	// layer name
	if (argData->isFlagSet(FlagNames::LAYERNAME)){
		if (!layerManager){
			throwStatusException("can't get layer name - no layers detected", MStatus::kFailure);
		}
		requireFlag(FlagNames::LAYERID,"provide layer ID to query name");
		SkinLayer * layer = getLayerFromArgId(FlagNames::LAYERID,"no layer under given ID");
		setResult(layer->getName());
		return MStatus::kSuccess;
	}

	// layer parent
	if (argData->isFlagSet(FlagNames::LAYERPARENT)){
		requireManager();
		
		// get layer either from flag, or use current layer
		SkinLayer * layer =  getOptionalLayer();
		if (layer!=NULL && layer->getParent())
			setResult(static_cast<int>(layer->getParent()->getID()));
		else
			setResult(SkinLayerManager::UNDEFINED_LAYER_ID);
		return MStatus::kSuccess;
	}


	// layer opacity
	if (argData->isFlagSet(FlagNames::LAYEROPACITY)){
		requireManager();
		SkinLayer * layer = requireLayer();
		setResult(layer->getOpacity());
		return MStatus::kSuccess;
	}

	// layer enabled
	if (argData->isFlagSet(FlagNames::LAYERENABLED)){
		requireManager();
		SkinLayer * layer = requireLayer();
		setResult(layer->isEnabled());
		return MStatus::kSuccess;
	}


	// layer data attach target
	if (selectedShape.isValid() && argData->isFlagSet(FlagNames::LAYERDATAATTACHTARGET)){
		MStringArray result;	
		MObject skinCluster = SkinLayerManager::findManagerAttachPoint(selectedShape);
		if (skinCluster!=MObject::kNullObj){
			result.append(selectedShape.partialPathName());
			result.append(MFnDependencyNode(skinCluster).name());
		}
		this->setResult(result);
		return MStatus::kSuccess;
	}

	// mirror cache info
	if (argData->isFlagSet(FlagNames::MIRRORCACHEINFO)) {
		const MString MSG_ERROR("error");
		const MString MSG_OK("ok");
		MStringArray result;
		
		if (!layerManager){
			result.append(MSG_ERROR);
			result.append("Skin layers not found in selection");
		}
		else
		if (layerManager->mirrorData.isInitialized()){
			result.append(MSG_OK);
			result.append("Mirror data is up to date");
		}
		else {
			result.append(MSG_ERROR);
			result.append("Mirror data is not initialized");
		}

		this->setResult(result);
		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::MIRRORAXIS)) {
		requireManager();
		const WeightTransferAssociation & data = layerManager->mirrorData;
		setResult(axisToString(data.getMirrorAxis()));
		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::MIRRORCACHEINFLUENCES)) {
		this->clearResult();
		if (!layerManager || !layerManager->mirrorData.isInitialized()){
			return MStatus::kSuccess;
		}

		const InfluenceTransferInfoVec &vec = layerManager->mirrorData.getInfluencesList();

		VECTOR_FOREACH_CONST(InfluenceTransferInfo *,vec,curr){
			if ((*curr)->getDestination()!=NULL){
				this->appendToResult((*curr)->getPath());
				this->appendToResult((*curr)->getDestination()->getPath());
				// is this a manual override?
				if (layerManager->mirrorManualOverrides.find((*curr)->getLogicalIndex())!=layerManager->mirrorManualOverrides.end()) {
					this->appendToResult(1);
				}
				else {
					this->appendToResult(0);
				}
			}
		}

		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::LAYERINDEX)) {
		requireManager();
		SkinLayer * layer = requireLayer();
		this->setResult(static_cast<int>(layer->getIndex()));
		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::NUM_CHILDREN)) {
		requireManager();
		SkinLayer * layer = requireLayer();
		if (layer!=NULL)
			this->setResult(static_cast<int>(layer->children.size()));
		else {
			this->setResult(0);
		}
		return MStatus::kSuccess;
	}


	// vertex weights
	if (argData->isFlagSet(FlagNames::VERTEXWEIGHTS) && layerManager && argData->isFlagSet(FlagNames::PAINTTARGET)){
		MDoubleArray result;
		result.setLength(0);
		SkinLayer * layer = requireLayer();

		MString ptFlagValue = argData->flagArgumentString(FlagNames::PAINTTARGET,0);
		DEBUG_COUT_ENDL("vertex weights - paint target: "<<ptFlagValue);
		
		if (ptFlagValue==MString(PaintTargetNames::MASK)){
			layer->maskWeightList.getWeights(result);
		}

		if (ptFlagValue==MString(PaintTargetNames::INFLUENCE) && argData->isFlagSet(FlagNames::INFLUENCEID)){
			unsigned int influence = static_cast<unsigned int>(argData->flagArgumentInt(FlagNames::INFLUENCEID,0));
			if (layer->influenceWeightList.hasLogicalInfluence(influence)) {
				layer->influenceWeightList.getLogicalInfluenceWeights(influence,result);
			}
		}

		this->setResult(result);
		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::VERTCOUNT)) {
		requireManager();
		this->setResult(static_cast<int>(layerManager->getMeshVertCount()));
		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::MIRROR_INFLUENCE_ASSOCIATION)){
		MStringArray result;
		queryManualMirrorInfluenceAssociations(result);
		setResult(result);
		return MStatus::kSuccess;
	}

	if (argData->isFlagSet(FlagNames::INFLUENCE_LIMIT_PER_VERTEX)) {
		requireManager();
		this->setResult(static_cast<int>(layerManager->getInfluenceLimitPerVert()));
		return MStatus::kSuccess;
	}


	// invalid query?
	return MStatus::kInvalidParameter;
}

void ngSkinLayerCmd::queryManualMirrorInfluenceAssociations(MStringArray &result){
	requireManager();
	
	for (std::map<unsigned int,unsigned int>::const_iterator it=layerManager->mirrorManualOverrides.begin();it!=layerManager->mirrorManualOverrides.end();it++){
		MDagPath path;
		layerManager->getInfluencePath(it->first,path);
		result.append(path.partialPathName());
		layerManager->getInfluencePath(it->second,path);
		result.append(path.partialPathName());
	}
}

SkinLayerChanges::SkinLayerChange * ngSkinLayerCmd::createUndoableBit(){
	// add layer
	if (layerManager && argData->isFlagSet(FlagNames::ADDLAYER)){
		requireFlag(FlagNames::LAYERNAME,"layer name is required when adding layer");

		SkinLayerChanges::AddLayer * change = new SkinLayerChanges::AddLayer();
		change->setForceEmpty(argData->isFlagSet(FlagNames::FORCEEMPTY) && argData->flagArgumentBool(FlagNames::FORCEEMPTY,0));

		argData->getFlagArgument(FlagNames::LAYERNAME,0,change->newName);
		if (argData->isFlagSet(FlagNames::LAYERPARENT))
			change->parent = getLayerFromArgId(FlagNames::LAYERPARENT,"no parent under given ID");
		else
			change->parent = layerManager->getLayerByID(0);

		return change;
	}

	// remove layer
	if (layerManager && argData->isFlagSet(FlagNames::REMOVELAYER)){
		SkinLayer * layer = requireLayer();
		SkinLayerChanges::RemoveLayer * change = new SkinLayerChanges::RemoveLayer();
		change->layer = layer;
		return change;
	}

	// rename layer
	if (layerManager && argData->isEdit() && argData->isFlagSet(FlagNames::LAYERNAME)){
		requireFlag(FlagNames::LAYERNAME,"new name was not specified");
		SkinLayer * layer = requireLayer();

		SkinLayerChanges::ChangeLayerName * change = new SkinLayerChanges::ChangeLayerName();
		change->newName = argData->flagArgumentString(FlagNames::LAYERNAME,0);
		change->layer = layer;
		return change;
	}

	// change layer opacity
	if (layerManager && argData->isEdit() && argData->isFlagSet(FlagNames::LAYEROPACITY)){
		requireFlag(FlagNames::LAYEROPACITY,"new opacity was not specified");
		SkinLayer *layer = requireLayer();

		SkinLayerChanges::ChangeOpacity * change = new SkinLayerChanges::ChangeOpacity();
		change->newOpacity = argData->flagArgumentDouble(FlagNames::LAYEROPACITY,0);
		change->layer = layer;
		
		return change;
	}

	// change layer enabled
	if (layerManager && argData->isEdit() && argData->isFlagSet(FlagNames::LAYERENABLED)){
		requireFlag(FlagNames::LAYERENABLED,"new opacity was not specified");
		SkinLayer *layer = requireLayer();
		
		SkinLayerChanges::ChangeEnabled * change = new SkinLayerChanges::ChangeEnabled();
		
		change->newEnabled = argData->flagArgumentBool(FlagNames::LAYERENABLED,0);
		DEBUG_COUT_ENDL("new enabled: "<<change->newEnabled);
		change->layer = layer;
		
		return change;
	}

	// change current layer
	if (argData->isFlagSet(FlagNames::CURRLAYER)){
		requireManager();
		requireFlag(FlagNames::CURRLAYER,"provide layer ID for new current layer");
		return new SkinLayerChanges::ChangeCurrentLayer(getLayerFromArgId(FlagNames::CURRLAYER,"no layer under given ID"));
	}

	// change current influence
	if (argData->isFlagSet(FlagNames::CURRINFLUENCE)){
		requireManager();
		requireLayer();
		requireFlag(FlagNames::CURRINFLUENCE,"provide influence ID");
		return new SkinLayerChanges::ChangeCurrentInfluence(argData->flagArgumentInt(FlagNames::CURRINFLUENCE,0));
	}

	// change current paint target
	if (argData->isFlagSet(FlagNames::CURRPAINTTARGET)){
		requireManager();
		MString cptFlagValue = argData->flagArgumentString(FlagNames::CURRPAINTTARGET,0);
		if (cptFlagValue==MString(PaintTargetNames::MASK)){
			return new SkinLayerChanges::ChangeCurrentInfluence(SkinLayer::PAINT_TARGET_MASK);
		}
		throwStatusException("invalid paint target name",MStatus::kInvalidParameter);
	}

	if (argData->isFlagSet(FlagNames::COLORDISPLAYNODE)){
		if (argData->flagArgumentBool(FlagNames::COLORDISPLAYNODE,0)){
			if (layerManager){
				return new SkinLayerChanges::AddDisplay();
			}
		}
		else {
			return new SkinLayerChanges::RemoveDisplay();
		}
	}


	// attach layer data
	if (selectedShape.isValid() && argData->isFlagSet(FlagNames::LAYERDATAATTACH)){
		DEBUG_COUT_ENDL("attaching data to "<<selectedShape.fullPathName());
		SkinLayerChanges::AttachData *change = new SkinLayerChanges::AttachData(selectedShape);
		return change;
	}

	// mirror layer weights
	if (argData->isFlagSet(FlagNames::MIRRORLAYERWEIGHTS) || argData->isFlagSet(FlagNames::MIRRORLAYERMASK)){
		requireManager();
		requireInitializedMirrorData();
		SkinLayer * targetLayer = requireLayer();

		SkinLayerChanges::MirrorLayer *change = new SkinLayerChanges::MirrorLayer(*targetLayer);

		change->mirrorWidth = argData->isFlagSet(FlagNames::MIRRORWIDTH)?argData->flagArgumentDouble(FlagNames::MIRRORWIDTH,0):0.0;
		change->mirrorMask = argData->isFlagSet(FlagNames::MIRRORLAYERMASK) && argData->flagArgumentBool(FlagNames::MIRRORLAYERMASK,0);
		change->mirrorWeights = argData->isFlagSet(FlagNames::MIRRORLAYERWEIGHTS) && argData->flagArgumentBool(FlagNames::MIRRORLAYERWEIGHTS,0);
		
		if (argData->isFlagSet(FlagNames::MIRRORDIRECTION)) {
			const int directionNegativeToPositive = 0;
			const int directionPositiveToNegative = 1;
			const int directionGuess = 2;
			int directionValue = argData->flagArgumentInt(FlagNames::MIRRORDIRECTION,0);
			change->positiveToNegative =  directionValue==directionPositiveToNegative;
			change->guessMirrorSide = directionValue==directionGuess;
		}
		return change;
	}

	if (argData->isFlagSet(FlagNames::PAINTFLOOD)){
		requireManager();
		SkinLayer * const layer = requireLayer();
		return new SkinLayerChanges::FloodWeights(*layer);
	}

	if (argData->isFlagSet(FlagNames::TRANSPARENCYTOMASK)){
		requireManager();
		SkinLayer * layer = requireLayer();
		return new SkinLayerChanges::TransparencyToMask(*layer);
	}
	if (argData->isFlagSet(FlagNames::MASKTOTRANSPARENCY)){
		requireManager();
		SkinLayer * layer = requireLayer();
		return new SkinLayerChanges::MaskToTransparency(*layer);
	}

	if (argData->isFlagSet(FlagNames::LAYERINDEX) && layerManager && layerManager->currentLayer){
		return new SkinLayerChanges::ChangeLayerIndex(*layerManager->currentLayer,argData->flagArgumentInt(FlagNames::LAYERINDEX,0));
	}

	// vertex weights
	if (argData->isFlagSet(FlagNames::VERTEXWEIGHTS) && argData->isFlagSet(FlagNames::PAINTTARGET)){
		requireManager();
		SkinLayer * layer = requireLayer();
		SkinLayerChanges::SetLayerWeights * change = new SkinLayerChanges::SetLayerWeights(*layer);
		MString ptFlagValue = argData->flagArgumentString(FlagNames::PAINTTARGET,0);
		
		MDoubleArray weights;
		mStringToDoubleArray(argData->flagArgumentString(FlagNames::VERTEXWEIGHTS,0),weights);
		change->setNewWeights(weights);
		
		if (ptFlagValue==MString(PaintTargetNames::MASK)){
			change->setTarget(SkinLayer::PAINT_TARGET_MASK);
		}

		if (ptFlagValue==MString(PaintTargetNames::INFLUENCE) && argData->isFlagSet(FlagNames::INFLUENCEID)){
			const int influenceId = argData->flagArgumentInt(FlagNames::INFLUENCEID,0);
			if (influenceId<0) {
				throwStatusException("invalid influence id",MStatus::kInvalidParameter);
			}
			change->setTarget(influenceId);
		}

		return change;
	}

	if (argData->isFlagSet(FlagNames::INFLUENCE_LIMIT_PER_VERTEX)){
		requireManager();
		return new SkinLayerChanges::SetMaxInfluencesPerVert(argData->flagArgumentInt(FlagNames::INFLUENCE_LIMIT_PER_VERTEX,0));
	}


	return NULL;
}


void ngSkinLayerCmd::initMirrorData(){
	RuleDescriptionList ruleList;
	ruleList.isMirrorMode = true;

	MatchDistanceRule distanceRule;
	MatchNameRule nameRule;
	MatchManualOverrideRule manualRule;
	
	ruleList.addRule(&manualRule);
	ruleList.addRule(&nameRule);
	ruleList.addRule(&distanceRule);


	for (std::map<unsigned int, unsigned int>::const_iterator i=layerManager->mirrorManualOverrides.begin();i!=layerManager->mirrorManualOverrides.end();i++){
		manualRule.addOverride(i->first,i->second);
	}

	if (argData->isFlagSet(FlagNames::MIRRORAXIS)){
		MString flagValue = argData->flagArgumentString(FlagNames::MIRRORAXIS,0);
		flagValue = flagValue.toLowerCase();

		// X by default, unless specified otherwise
		if (flagValue=="y"){
			ruleList.setMirrorAxis(SkinToolsGlobal::Y);
		}
		else if (flagValue=="z"){
			ruleList.setMirrorAxis(SkinToolsGlobal::Z);
		}
		else{
			ruleList.setMirrorAxis(SkinToolsGlobal::X);
		}
		DEBUG_COUT_ENDL("mirror axis: "<<flagValue);
	}


	if (argData->isFlagSet(FlagNames::INFLUENCEASSOCIATIONDISTANCE)){
		distanceRule.setThreshold(argData->flagArgumentDouble(FlagNames::INFLUENCEASSOCIATIONDISTANCE,0));
	}
	else {
		distanceRule.setThreshold(SMALL_NUMBER_LAMBDA);
	}

	// prefixes
	if (argData->isFlagSet(FlagNames::INFLUENCEPREFIX)){
		MString prefixes = argData->flagArgumentString(FlagNames::INFLUENCEPREFIX,0);
		MStringArray prefixesList;
		prefixes.split(',',prefixesList);
		for (unsigned int i=0;i<prefixesList.length();i++){
			DEBUG_COUT_ENDL("parsed prefix '"<<prefixesList[i]<<"'");
			nameRule.addPrefix(prefixesList[i]);
		}
	}
	
	
	layerManager->initSkinMirrorData(ruleList);

	DEBUG_EXECUTE(layerManager->mirrorData.dumpInfluenceAssociations());
}

void ngSkinLayerCmd::detectLayerManager(){
	MStatus status;
	layerManager = NULL;

	if (getSelectedObjects().length()>0){
		getSelectedObjects().getDagPath(0,selectedShape);
		layerManager = SkinLayerManager::findManager(selectedShape);
	}
}

MStatus ngSkinLayerCmd::doIt( const MArgList& args){
	try{
		MStatus status;
		MArgDatabase argData( syntax(), args, &status );
		this->argData = &argData;
		CHECK_STATUS("initialize arg data",status);

		status = argData.getObjects(this->selectedObjects);
		CHECK_STATUS("arg data: get objects",status);

		detectLayerManager();


		if (argData.isQuery()){
			return this->handleQuery();
		}

		
		// TODO: dispatch following section into query/undoable commands
		{
			if ( argData.isFlagSet(FlagNames::DISPLAY_UPDATE)){
				if (layerManager)
					layerManager->invalidateDisplay();
			}

			if ( argData.isFlagSet(FlagNames::PAINTINGMODE)){
				SkinLayerManager::isPainting = argData.flagArgumentBool(FlagNames::PAINTINGMODE,0);
				SkinLayerManager::displayColorOnSelection(SkinLayerManager::isPainting);
			}

			if (argData.isFlagSet(FlagNames::PAINTOPERATION)){
				SkinLayerPaintGlobals::currentPaintMode = static_cast<PaintMode>(argData.flagArgumentInt(FlagNames::PAINTOPERATION,0));
				DEBUG_COUT_ENDL("changed paint mode to "<<SkinLayerPaintGlobals::modeName(SkinLayerPaintGlobals::currentPaintMode));
			}

			if (argData.isFlagSet(FlagNames::PAINTINTENSITY)){
				SkinLayerPaintGlobals::setIntensity(argData.flagArgumentDouble(FlagNames::PAINTINTENSITY,0));
			}

			if (argData.isFlagSet(FlagNames::INITMIRRORDATA)){
				requireManager();
				this->initMirrorData();
			}

			// nearly identical code for add/remove influence assoc
			if (argData.isFlagSet(FlagNames::MIRROR_INFLUENCE_ASSOCIATION) || argData.isFlagSet(FlagNames::REMOVE_MIRROR_INFLUENCE_ASSOCIATION)) {
				requireManager();


				MStringArray influences;
				bool add = argData.isFlagSet(FlagNames::MIRROR_INFLUENCE_ASSOCIATION);
				MString flag = argData.flagArgumentString(add?FlagNames::MIRROR_INFLUENCE_ASSOCIATION:FlagNames::REMOVE_MIRROR_INFLUENCE_ASSOCIATION,0);
				flag.split(',',influences);
				if (influences.length()!=2){
					return MStatus::kInvalidParameter;
				}

				
				bool result;
				if (add) {
					result = layerManager->addMirrorInfluenceAssociation(influences[0],influences[1]);
				}
				else {
					result = layerManager->removeMirrorInfluenceAssociation(influences[0],influences[1]);
				}
				if (!result)
					return MStatus::kFailure;
			}


			if (argData.isFlagSet(FlagNames::COPYSKINDATA) && layerManager){
				DEBUG_COUT_ENDL("-----TOTAL OBJECTS SELECTED: "<<getSelectedObjects().length());
		
				if (getSelectedObjects().length()>1){
					MDagPath secondMesh;
					getSelectedObjects().getDagPath(1,secondMesh);
					SkinLayerManager * targetLayerManager = SkinLayerManager::findManager(secondMesh);

					if (targetLayerManager && (targetLayerManager!=layerManager)){
						targetLayerManager->transferWeights(*layerManager);
					}
				}
			}

			if (argData.isFlagSet(FlagNames::BEGINDATAUPDATE)) {
				requireManager();
				layerManager->delayedUpdatesState.suspend();
			}

			if (argData.isFlagSet(FlagNames::ENDDATAUPDATE)) {
				requireManager();
				
				if (!layerManager->delayedUpdatesState.isSuspended()){
					throwStatusException("cannot unsuspend: layer manager is not suspended",MStatus::kInvalidParameter);
				}

				layerManager->delayedUpdatesState.unsuspend();
			}


		}

		status = MS::kSuccess;
		this->change = this->createUndoableBit();
		
		// so, we've got undoable change ready?
		if (this->change) {
			this->change->manager = layerManager;
			this->change->execute();
			this->change->setResult(*this);
		}
	}
	catch (StatusException &e){
		DEBUG_EXECUTE(MGlobal::displayError(MString("ngSkinLayerCmd failed:")+e.getStatus().errorString()));
		return e.getStatus();
	}
	return MStatus::kSuccess;
}

MStatus ngSkinLayerCmd::redoIt()
{
	try{
		if (this->change)
			this->change->redo();
	}
	catch (StatusException &e){
		return e.getStatus();
	}
	return MS::kSuccess;
}

MStatus ngSkinLayerCmd::undoIt()
{
	if (this->change)
		this->change->undo();
	return MS::kSuccess;
}

void* ngSkinLayerCmd::creator()
{
	return new ngSkinLayerCmd();
}

bool ngSkinLayerCmd::isUndoable() const
{
	return (change!=NULL);
}

MSyntax ngSkinLayerCmd::syntaxCreator(){
	MSyntax result;
	result.useSelectionAsDefault(true);
	result.setObjectType(MSyntax::kSelectionList);

	result.addFlag(FlagNames::ADDLAYER, "-addLayer");
	result.addFlag(FlagNames::FORCEEMPTY, "-forceEmpty", MSyntax::kBoolean);
	result.addFlag(FlagNames::REMOVELAYER, "-removeLayer");
	result.addFlag(FlagNames::LAYERNAME, "-name",MSyntax::kString);
	result.addFlag(FlagNames::LAYEROPACITY, "-opacity", MSyntax::kDouble);
	result.addFlag(FlagNames::LAYERENABLED, "-enabled", MSyntax::kBoolean);
	result.addFlag(FlagNames::LISTLAYERS, "-listLayers");
	result.addFlag(FlagNames::LISTLAYERINFLUENCES, "-listLayerInfluences");
	result.addFlag(FlagNames::ACTIVEINFLUENCES, "-activeInfluences");
	result.addFlag(FlagNames::LAYERID, "-layerId",MSyntax::kLong);
	result.addFlag(FlagNames::LAYERPARENT, "-parent",MSyntax::kLong);
	result.addFlag(FlagNames::CURRLAYER, "-currentLayer",MSyntax::kLong);
	result.addFlag(FlagNames::CURRINFLUENCE, "-currentInfluence",MSyntax::kLong);
	result.addFlag(FlagNames::CURRPAINTTARGET, "-currentPaintTarget",MSyntax::kString);
	result.addFlag(FlagNames::DISPLAY_UPDATE, "-displayUpdate");
	result.addFlag(FlagNames::PAINTINGMODE, "-paintingMode",MSyntax::kBoolean);
	result.addFlag(FlagNames::COLORDISPLAYNODE, "-colorDisplayNode",MSyntax::kBoolean);

	result.addFlag(FlagNames::LAYERDATAATTACH, "-layerDataAttach");
	result.addFlag(FlagNames::LAYERDATAATTACHTARGET,"-layerDataAttachTarget");

	result.addFlag(FlagNames::INITMIRRORDATA,"-initMirrorData");
	result.addFlag(FlagNames::MIRRORLAYERWEIGHTS, "-mirrorLayerWeights",MSyntax::kBoolean);
	result.addFlag(FlagNames::MIRRORLAYERMASK, "-mirrorLayerMask",MSyntax::kBoolean);
	result.addFlag(FlagNames::MIRRORWIDTH,"-mirrorWidth",MSyntax::kDouble);
	result.addFlag(FlagNames::MIRRORAXIS,"-mirrorAxis",MSyntax::kString);
	result.addFlag(FlagNames::MIRRORDIRECTION,"-mirrorDirection",MSyntax::kUnsigned);
	result.addFlag(FlagNames::MIRRORCACHEINFO,"-mirrorCacheInfo");
	result.addFlag(FlagNames::MIRRORCACHEINFLUENCES,"-mirrorCacheInfluences");
	
	result.addFlag(FlagNames::INFLUENCEASSOCIATIONDISTANCE,"-influenceAssociationDistance",MSyntax::kDouble);
	result.addFlag(FlagNames::INFLUENCEPREFIX,"-influenceAssociationPrefix",MSyntax::kString);

	CHECK_STATUS("could not add flag 'mirrorInfluenceAssociation'",result.addFlag(FlagNames::MIRROR_INFLUENCE_ASSOCIATION,"-mirrorInfluenceAssociation",MSyntax::kString));
	result.addFlag(FlagNames::REMOVE_MIRROR_INFLUENCE_ASSOCIATION,"-removeMirrorInfluenceAssociation",MSyntax::kString);

	result.addFlag(FlagNames::COPYSKINDATA,"-copySkinData");

	result.addFlag(FlagNames::PAINTFLOOD,"-paintFlood");
	result.addFlag(FlagNames::PAINTOPERATION,"-paintOperation",MSyntax::kLong);
	result.addFlag(FlagNames::PAINTINTENSITY,"-paintIntensity",MSyntax::kDouble);

	result.addFlag(FlagNames::TRANSPARENCYTOMASK,"-transparencyToMask");
	result.addFlag(FlagNames::MASKTOTRANSPARENCY,"-maskToTransparency");

	result.addFlag(FlagNames::LAYERINDEX,"-layerIndex",MSyntax::kLong);
	result.addFlag(FlagNames::NUM_CHILDREN,"-numChildren",MSyntax::kLong);

	result.addFlag(FlagNames::INFLUENCEID,"-influenceId",MSyntax::kLong);
	result.addFlag(FlagNames::PAINTTARGET,"-paintTarget",MSyntax::kString);
	result.addFlag(FlagNames::VERTEXWEIGHTS,"-vertexWeights",MSyntax::kString);

	result.addFlag(FlagNames::VERTCOUNT,"-vertexCount",MSyntax::kLong);
	
	result.addFlag(FlagNames::BEGINDATAUPDATE,"-beginDataUpdate");
	result.addFlag(FlagNames::ENDDATAUPDATE,"-endDataUpdate");

	result.addFlag(FlagNames::INFLUENCE_LIMIT_PER_VERTEX,"-influenceLimitPerVertex",MSyntax::kLong);
	

	result.enableQuery(true);
	result.enableEdit(true);

	return result;
}
