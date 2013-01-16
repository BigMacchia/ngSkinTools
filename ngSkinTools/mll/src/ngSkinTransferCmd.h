#pragma once
#include "maya.h"

class ngSkinTransferCmd: public MPxCommand
{
public:
	static const char COMMAND_NAME[];

	ngSkinTransferCmd(void);
	~ngSkinTransferCmd(void);

	static		void* creator();
	static		MSyntax syntaxCreator();
	

	MStatus doIt( const MArgList& );
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;

};
