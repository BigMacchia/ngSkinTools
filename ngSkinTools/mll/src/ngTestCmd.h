#pragma once
#ifdef _DEBUG

#include "maya.h"

const char COMMAND_NGTESTCMD_NAME[] = "ngTestCmd";

/**
 * this is a command only defined in a debug mode of the plugin,
 * who's sole purpose is randomly running various test of a features
 * in progress
*/
class ngTestCmd : public MPxCommand
{
public:
	static MCallbackIdArray callbacks;
	static void testHookSkinClusterChange(MSelectionList &list);

	static		void* creator();
	static MSyntax newSyntax();
	virtual bool hasSyntax();


	ngTestCmd();
	virtual ~ngTestCmd();

	MStatus doIt( const MArgList& );
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;

	void pointCloudTest(MSelectionList selList,double radius);

	// uninitialize any callback that was present
	static void removeCallbacks(){
		if (callbacks.length())
			MNodeMessage::removeCallbacks(callbacks);
	}
};

#endif // debug
