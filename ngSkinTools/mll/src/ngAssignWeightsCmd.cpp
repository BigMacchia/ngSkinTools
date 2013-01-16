#include <assert.h>

#include "ngAssignWeightsCmd.h"
#include "StatusException.h"
#include "utils.h"
#include "WeightsByClosestJoint.h"
#include "MakeRigidWeights.h"
#include "LimitWeights.h"
#include "ProgressWindow.h"
//#include "WeightsByClosestJointDelaunay.h"

const char ngAssignWeightsCmd::COMMAND_NAME[] = "ngAssignWeights";

namespace AssignWeights {
	namespace FlagNames {
		const char PRESERVE_LOCKED_INFLUENCES[] = "-pli";
		const char INTENSITY[] = "-i";
		const char SOFT_SELECTION_RADIUS[] = "-ssr";
		const char NATIVE_SOFT_SELECTION[] = "-nss";

		const char MAKERIGID[] = "-mr";
		const char SINGLE_CLUSTER[] = "-sc";

		const char BYNEARESTJOINT[] = "-bnj";
		const char USE_INTERSECTIONS[] = "-isc";
		const char INCLUDEJOINTS[] = "-ij";

		const char LIMITWEIGHTS[] = "-lw";
		const char LIMIT_AMOUNT[] = "-la";

		const char BYNEARESTJOINTD[] = "-njd";
	}
}
using namespace AssignWeights;


ngAssignWeightsCmd::ngAssignWeightsCmd(void){
	this->engine = NULL;
}

ngAssignWeightsCmd::~ngAssignWeightsCmd(void){
	if (engine)
		delete engine;
}

void* ngAssignWeightsCmd::creator(){
	return new ngAssignWeightsCmd();
}

MSyntax ngAssignWeightsCmd::syntaxCreator(){
	MSyntax result;
	result.useSelectionAsDefault(true);
	result.setObjectType(MSyntax::kSelectionList,1);

	// general flags
	result.addFlag(FlagNames::PRESERVE_LOCKED_INFLUENCES,"-preserveLockedInfluences");
	result.addFlag(FlagNames::INTENSITY,"-intensity",MSyntax::kDouble);
	result.addFlag(FlagNames::SOFT_SELECTION_RADIUS,"-softSelectionRadius",MSyntax::kDouble);
	result.addFlag(FlagNames::NATIVE_SOFT_SELECTION, "-nativeSoftSelection", MSyntax::kBoolean);

	// nearest joint specific flags
	result.addFlag(FlagNames::BYNEARESTJOINT,"-byNearestJoint");
	result.addFlag(FlagNames::USE_INTERSECTIONS,"-useIntersections");
	result.addFlag(FlagNames::INCLUDEJOINTS,"-includeJoints",MSyntax::kString);

	// make rigid specific flags
	result.addFlag(FlagNames::MAKERIGID,"-makeRigid");
	result.addFlag(FlagNames::SINGLE_CLUSTER,"-singleCluster");

	// limit weights specific flags
	result.addFlag(FlagNames::LIMITWEIGHTS,"-limitWeights");
	result.addFlag(FlagNames::LIMIT_AMOUNT,"-limitAmount",MSyntax::kUnsigned);

	// by nearest delaunay
	result.addFlag(FlagNames::BYNEARESTJOINTD,"-byNearestJointD");

	return result;
}

bool ngAssignWeightsCmd::isUndoable() const {
	return ((this->engine!=NULL)&&this->engine->canUndo());
}

MStatus ngAssignWeightsCmd::doIt( const MArgList& args ){

	try{
		MStatus status;
		MArgDatabase argData( syntax(), args, &status );
		CHECK_STATUS("initialize arg data",status);
		
		if (argData.isFlagSet(FlagNames::BYNEARESTJOINT)){
			WeightsByClosestJoint * const closestJointEngine = new WeightsByClosestJoint();
			this->engine = closestJointEngine;
			closestJointEngine->useIntersectionRanking = argData.isFlagSet(FlagNames::USE_INTERSECTIONS);

			if (argData.isFlagSet(FlagNames::INCLUDEJOINTS)){
				Utils::getSelectionFromString(argData.flagArgumentString(FlagNames::INCLUDEJOINTS,0),closestJointEngine->includedInfluences);
			}
		}
		else
		if (argData.isFlagSet(FlagNames::MAKERIGID)){
			MakeRigidWeights * const rigidWeightsEngine = new MakeRigidWeights();
			this->engine = rigidWeightsEngine;


			rigidWeightsEngine->isSingleClusterMode = argData.isFlagSet(FlagNames::SINGLE_CLUSTER);
			if (rigidWeightsEngine->isSingleClusterMode){
				DEBUG_COUT_ENDL("using single cluster mode");
			}
			else {
				DEBUG_COUT_ENDL("not using single cluster mode");
			}

		}
/*		else
		if (argData.isFlagSet(FlagNames::BYNEARESTJOINTD)){
			WeightsByClosestJointDelaunay * e = new WeightsByClosestJointDelaunay();
			this->engine = e;
		}*/
		else
		if (argData.isFlagSet(FlagNames::LIMITWEIGHTS)){
			LimitWeights * limitWeightsEngine = new LimitWeights();
			this->engine = limitWeightsEngine;
			if (argData.isFlagSet(FlagNames::LIMIT_AMOUNT)){
				limitWeightsEngine->numInflLimit = argData.flagArgumentInt(FlagNames::LIMIT_AMOUNT,0,&status);
				CHECK_STATUS("reading limit amount",status);
			}
		}

		if (!this->engine){
			this->displayError("weights assigning method was not specified");
			return MS::kInvalidParameter;
		}

		status = argData.getObjects(this->engine->vertSelection);
		CHECK_STATUS("arg data: get objects",status);
		this->engine->preserveLockedInfluences = argData.isFlagSet(FlagNames::PRESERVE_LOCKED_INFLUENCES);
		
		if (argData.isFlagSet(FlagNames::INTENSITY)){
			this->engine->setIntensity(argData.flagArgumentDouble(FlagNames::INTENSITY,0,&status));
			CHECK_STATUS("reading intensity",status);
		}
		if (argData.isFlagSet(FlagNames::SOFT_SELECTION_RADIUS)){
			this->engine->softSelectionRadius = argData.flagArgumentDouble(FlagNames::SOFT_SELECTION_RADIUS,0,&status);
			CHECK_STATUS("reading soft selection radius",status);
		}
		if (argData.isFlagSet(FlagNames::NATIVE_SOFT_SELECTION)) {
			this->engine->useExternalSoftSelection = argData.flagArgumentBool(FlagNames::NATIVE_SOFT_SELECTION,0,&status);
			CHECK_STATUS("reading native soft selection flag",status);
		}



		this->engine->execute();

	}
	catch (StatusException &e){
		return e.getStatus();
	}

	return redoIt();
}

MStatus ngAssignWeightsCmd::redoIt(){
	try{
		assert(this->engine);
		this->engine->writeSkinWeights();
	}
	catch (StatusException &e){
		return e.getStatus();
	}

	return MS::kSuccess;
}

MStatus ngAssignWeightsCmd::undoIt(){
	try{
		assert(this->engine);
		this->engine->writeOldSkinWeights();
	}
	catch (StatusException &e){
		return e.getStatus();
	}

	return MS::kSuccess;
}
