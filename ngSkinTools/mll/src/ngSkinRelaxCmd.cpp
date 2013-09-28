#include "ngSkinRelaxCmd.h"
#include "StatusException.h"
#include "maya.h"
#include <algorithm>

namespace SkinRelax{
	namespace FlagNames {
		const char NUMSTEPS[] = "-ns";
		const char STEPSIZE[] = "-ss";
		const char PRESERVE_LOCKED_INFLUENCES[] = "-pli";
		const char LOCKED_VERTS[] = "-lv";
		const char INVISIBLE_VERTS[] = "-iv";
		const char ASSOCIATE_VOLUME[] = "-abv";
		const char ASSOCIATE_VOLUME_RADIUS[] = "-avr";
		const char SOFT_SELECTION_RADIUS[] = "-ssr";
		const char NATIVE_SOFT_SELECTION[] = "-nss";
		const char WEIGHT_LIMIT[] = "-wl";
	}
}
using namespace SkinRelax;
using namespace std;

const char ngSkinRelax::COMMAND_NAME[] = "ngSkinRelax";


MStatus ngSkinRelax::doIt( const MArgList& args)
{
	DEBUG_COUT_ENDL("--ngSkinRelax-----");
	try{
		this->readArgs(args);
		this->relaxEngine.execute();
	}
	catch (StatusException &e){
		return e.getStatus();
	}
	return redoIt();
}

bool readMultiUseFlag(const char flagName[], MArgDatabase &args, MSelectionList &sel){
	if (!args.isFlagSet(flagName))
		return false;
	MStatus status;

    for(unsigned int i=0,numUses = args.numberOfFlagUses(flagName); i<numUses; i++ )
    {
            MArgList argList;
            status = args.getFlagArgumentList( flagName, i, argList );
			CHECK_STATUS("problem reading multi flag",status);

            MString name = argList.asString( 0, &status );

			CHECK_STATUS("problem reading multi flag (2)",status);

			status = sel.add(name);
			CHECK_STATUS("problem adding item to selection",status);
	}

	return true;
}

/**
 * parses argument list and stores data into local class variables
 */
void ngSkinRelax::readArgs(const MArgList & args){

	// parse args
	MStatus status;
	MArgDatabase argData( syntax(), args, &status );
	CHECK_STATUS("initialize arg data",status);

	// read vertex selection
	status = argData.getObjects(this->relaxEngine.vertSelection);
	CHECK_STATUS("arg data: get objects",status);

	if (argData.isFlagSet(FlagNames::STEPSIZE)) {
		this->relaxEngine.stepSize = min(1.0,argData.flagArgumentDouble(FlagNames::STEPSIZE,0,&status));
		
	}
		
	CHECK_STATUS("get step size",status);

	if (argData.isFlagSet(FlagNames::NUMSTEPS))
		this->relaxEngine.numSteps = argData.flagArgumentInt(FlagNames::NUMSTEPS,0,&status);
	CHECK_STATUS("get num steps",status);

	if (argData.isFlagSet(FlagNames::PRESERVE_LOCKED_INFLUENCES))
		this->relaxEngine.preserveLockedInfluences = argData.flagArgumentBool(FlagNames::PRESERVE_LOCKED_INFLUENCES,0,&status);
	CHECK_STATUS("reading PLI flag",status);

	if (argData.isFlagSet(FlagNames::NATIVE_SOFT_SELECTION)){
		this->relaxEngine.useExternalSoftSelection = argData.flagArgumentBool(FlagNames::NATIVE_SOFT_SELECTION,0,&status);
		CHECK_STATUS("reading native soft selection flag",status);
	}

	if (argData.isFlagSet(FlagNames::ASSOCIATE_VOLUME)){
		if (argData.flagArgumentBool(FlagNames::ASSOCIATE_VOLUME,0) && argData.isFlagSet(FlagNames::ASSOCIATE_VOLUME_RADIUS)){
			this->relaxEngine.volumeAssociationRadius = argData.flagArgumentDouble(FlagNames::ASSOCIATE_VOLUME_RADIUS,0,&status);
			CHECK_STATUS("associate volume radius",status);
		}
	}

	if (argData.isFlagSet(FlagNames::SOFT_SELECTION_RADIUS))
		this->relaxEngine.softSelectionRadius = argData.flagArgumentDouble(FlagNames::SOFT_SELECTION_RADIUS,0,&status);

	if (argData.isFlagSet(FlagNames::WEIGHT_LIMIT)) {
		this->relaxEngine.weightLimitEnabled = true;
		this->relaxEngine.weightsLimiter.hardLimit = argData.flagArgumentInt(FlagNames::WEIGHT_LIMIT,0,&status);
	}
	CHECK_STATUS("soft selection radius",status);


	MSelectionList lockedVerts;
	readMultiUseFlag(FlagNames::LOCKED_VERTS,argData,lockedVerts);

	readMultiUseFlag(FlagNames::INVISIBLE_VERTS,argData,this->relaxEngine.invisibleVerts);

}


MStatus ngSkinRelax::redoIt()
{
	try{
		this->relaxEngine.writeSkinWeights();
	}
	catch (StatusException &e){
		return e.getStatus();
	}

	return MS::kSuccess;
}

MStatus ngSkinRelax::undoIt()
{
	try{
		this->relaxEngine.writeOldSkinWeights();
	}
	catch (StatusException &e){
		return e.getStatus();
	}

	return MS::kSuccess;
}

void* ngSkinRelax::creator()
{
	return new ngSkinRelax();
}

ngSkinRelax::ngSkinRelax()
{}

ngSkinRelax::~ngSkinRelax()
{
}

bool ngSkinRelax::isUndoable() const
{
	return this->relaxEngine.canUndo();
}

MSyntax ngSkinRelax::syntaxCreator(){
	MSyntax result;
	result.useSelectionAsDefault(true);
	result.setObjectType(MSyntax::kSelectionList,1);

	result.addFlag(FlagNames::NUMSTEPS, "-numSteps", MSyntax::kLong);
	result.addFlag(FlagNames::STEPSIZE, "-stepSize", MSyntax::kDouble);
	result.addFlag(FlagNames::PRESERVE_LOCKED_INFLUENCES, "-preserveLockedInfluences", MSyntax::kBoolean);
	result.addFlag(FlagNames::ASSOCIATE_VOLUME, "-associateByVolume", MSyntax::kBoolean);
	result.addFlag(FlagNames::ASSOCIATE_VOLUME_RADIUS, "-associateByVolumeRadius", MSyntax::kDouble);
	result.addFlag(FlagNames::SOFT_SELECTION_RADIUS, "-softSelectionRadius", MSyntax::kDouble);
	result.addFlag(FlagNames::NATIVE_SOFT_SELECTION, "-nativeSoftSelection", MSyntax::kBoolean);
	result.addFlag(FlagNames::WEIGHT_LIMIT, "-weightLimit", MSyntax::kLong);

	result.addFlag(FlagNames::LOCKED_VERTS, "-lockedVert", MSyntax::kSelectionItem);
	result.makeFlagMultiUse(FlagNames::LOCKED_VERTS);
	result.addFlag(FlagNames::INVISIBLE_VERTS, "-invisibleVert", MSyntax::kSelectionItem);
	result.makeFlagMultiUse(FlagNames::INVISIBLE_VERTS);

	return result;
}
