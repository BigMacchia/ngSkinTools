#include "ngSkinTransferCmd.h"


const char ngSkinTransferCmd::COMMAND_NAME[] = "ngSkinTransfer";


ngSkinTransferCmd::ngSkinTransferCmd(void)
{
}

ngSkinTransferCmd::~ngSkinTransferCmd(void)
{
}

MStatus ngSkinTransferCmd::doIt( const MArgList& args){

	return MStatus::kSuccess;
}

MStatus ngSkinTransferCmd::redoIt()

{
	return MS::kSuccess;
}

MStatus ngSkinTransferCmd::undoIt()
{
	return MS::kSuccess;
}

void* ngSkinTransferCmd::creator()
{
	return new ngSkinTransferCmd();
}

bool ngSkinTransferCmd::isUndoable() const
{
	return true;
}

MSyntax ngSkinTransferCmd::syntaxCreator(){
	MSyntax result;
	result.useSelectionAsDefault(true);
	result.setObjectType(MSyntax::kSelectionList,1,2);

	result.enableQuery(true);
	result.enableEdit(true);

	return result;
}
