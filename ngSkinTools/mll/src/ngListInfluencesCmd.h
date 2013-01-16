#pragma once

#include "maya.h"

namespace ListInfluences {


	// TODO: documentation missing
	/**
	*/
	class ngListInfluencesCmd: public MPxCommand
	{
	private:
	public:
		static const char COMMAND_NAME[];
		static		void* creator();
		static MSyntax newSyntax();

		virtual bool hasSyntax();
		static MSyntax syntaxCreator();


		ngListInfluencesCmd();
		virtual ~ngListInfluencesCmd();

		MStatus doIt( const MArgList& );
		MStatus redoIt();
		MStatus undoIt();
		bool isUndoable() const;

	};

}
