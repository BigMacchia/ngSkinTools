#pragma once

#include "maya.h"
#include "RelaxEngine.h"



/**
 * \brief Defines ngSkinRelax command, as required by Maya API.
 *
 * Responsible for parsing arguments, grouping behaviour into do/undo/redo
 * and serving as a bridge to maya.
 * 
 * All the hard work is further passed to RelaxEngine.
 */
class ngSkinRelax : public MPxCommand
{

private:
	RelaxEngine relaxEngine;
	void readArgs(const MArgList&);

public:
	static const char COMMAND_NAME[];

	static		void* creator();
	static		MSyntax syntaxCreator();
	

	ngSkinRelax();
	MStatus doIt( const MArgList& );
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;



	virtual ~ngSkinRelax();

	
};
