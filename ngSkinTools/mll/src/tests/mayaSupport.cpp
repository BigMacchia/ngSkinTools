#include "mayaSupport.h"
#include <maya/MLibrary.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>

#undef ERROR
#include "glog/logging.h"


bool mayaLibraryLoaded = false;

bool setupMayaLibrary(){
	if (mayaLibraryLoaded)
		return true;


	LOG(INFO) << "initializing maya environment...";
    MStatus status;
    status = MLibrary::initialize (true, "testCases.exe", true);
    if ( !status ) {
        status.perror("MLibrary::initialize");
		LOG(ERROR) << "failed to initialize maya environment";
        return false;
    }
	LOG(INFO) << "maya environment initialized";
	
	MGlobal::executeCommand("unloadPlugin \"ngSkinTools\"");

	mayaLibraryLoaded = true;
	return true;
}