/**
 * shortcut module that contains all maya API related includes
 */

#pragma once

#include <maya/MPxCommand.h>

#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MObjectHandle.h>
#include <maya/MGlobal.h>
#include <maya/MPlug.h>

#include <maya/MSelectionList.h>

#include <maya/MFnMesh.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnTransform.h>

#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>


#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatPoint.h>

#include <maya/MItMeshVertex.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDependencyGraph.h>

#include <maya/MArgDatabase.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>

#include <maya/MNodeMessage.h>
#include <maya/MCallbackIdArray.h>

#if MAYA_API_VERSION>200800
#include <maya/MRichSelection.h>
#endif
