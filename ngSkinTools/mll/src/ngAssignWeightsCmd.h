#pragma once

#include "maya.h"
#include "AssignWeightsEngineBase.h"


class ngAssignWeightsCmd: public MPxCommand
{
private:
	AssignWeightsEngineBase * engine;
public:
	static const char COMMAND_NAME[];

	static		void* creator();
	static		MSyntax syntaxCreator();

	ngAssignWeightsCmd(void);
	virtual ~ngAssignWeightsCmd(void);

	bool isUndoable() const;
	MStatus doIt( const MArgList& );
	MStatus redoIt();
	MStatus undoIt();

};
