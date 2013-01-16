#include "ngListInfluencesCmd.h"
#include "StatusException.h"
#include "InflListerByWeights.h"
#include "InflListerBase.h"
#include "InflListerByProximity.h"


namespace ListInfluences {
	namespace FlagNames {
		const char INCLUDEWEIGHTINGINFO[] = "-iwi";
		const char LONGNAMES[] = "-ln";
		const char LISTBYWEIGHTS[] = "-lbw";
		const char LISTBYCLOSESTJOINT[] = "-lcj";
	}
}
using namespace ListInfluences;


const char ngListInfluencesCmd::COMMAND_NAME[] = "ngListInfluences";


void * ngListInfluencesCmd::creator(){
	return new ngListInfluencesCmd();
}

ngListInfluencesCmd::ngListInfluencesCmd()
{}

ngListInfluencesCmd::~ngListInfluencesCmd()
{
}

bool ngListInfluencesCmd::isUndoable() const
{
	return false;
}

MSyntax ngListInfluencesCmd::syntaxCreator(){
	MSyntax result;
	result.useSelectionAsDefault(true);
	result.setObjectType(MSyntax::kSelectionList,1);

	result.addFlag(FlagNames::LONGNAMES, "-longNames", MSyntax::kBoolean);

	result.addFlag(FlagNames::INCLUDEWEIGHTINGINFO, "-includeWeightingInfo", MSyntax::kBoolean);

	result.addFlag(FlagNames::LISTBYCLOSESTJOINT, "-listByClosestJoint",MSyntax::kBoolean);
	result.addFlag(FlagNames::LISTBYWEIGHTS, "-listByWeights",MSyntax::kBoolean);

	return result;
}

bool ngListInfluencesCmd::hasSyntax() {
	return true;
}

/**
 * returns true only if flag is set in argument database and it's set to "true"
 */
inline bool boolFlagTest(const MArgDatabase &argData, const char * flagName){
	return argData.isFlagSet(flagName) && argData.flagArgumentBool(flagName,0);
}

MStatus ngListInfluencesCmd::doIt( const MArgList& args){

	MStatus status;
	
	try {
		MArgDatabase argData( syntax(), args, &status );
		CHECK_STATUS("initialize arg data",status);
		
		InflListerBase * lister=NULL;

		if (boolFlagTest(argData,FlagNames::LISTBYCLOSESTJOINT)){
			lister = new InflListerByProximity();
		}
		

		// default lister: by weights
		if (!lister)
			lister = new InflListerByWeights();

		argData.getObjects(lister->vertSelection);


		lister->execute();

		
		MStringArray strings;
		// boolFlagTest(argData,FlagNames::SORTBYTOTALWEIGHTS)
		//boolFlagTest(argData,FlagNames::INCLUDEWEIGHTINGINFO)
		lister->getResult(strings,boolFlagTest(argData,FlagNames::LONGNAMES));

		this->setResult(strings);

		delete lister;
		return MS::kSuccess;

	}
	catch (StatusException &e){
		return e.getStatus();
	}


	return MS::kSuccess;
}

MStatus ngListInfluencesCmd::redoIt() {
	return MS::kSuccess;
}

MStatus ngListInfluencesCmd::undoIt() {
	return MS::kSuccess;
}

