#include <maya/MFnCompoundAttribute.h>
#include "utils.h"
#include "maya.h"
#include "StatusException.h"

#ifdef WIN32
#include "Windows.h"
#endif

bool Utils::saferExtendToShape(MDagPath &path){
	if (!path.hasFn(MFn::kShape)){
		// no shapes in this dag path node for sure
		return false;
	}

	// path already is shape?
	if (path.node().hasFn(MFn::kShape))
		return true;

	// extend to shape is required
	unsigned int numShapes = 0;
	path.numberOfShapesDirectlyBelow(numShapes);
	// should not happen, we already checked,
	// but just in case no shapes for this transform
	if (numShapes==0)
		return false;

	// extend to first shape that is not hidden from user (not an intermediate object)
	for (unsigned int i=0;i<numShapes;i++){
		MDagPath shape = path;
		shape.extendToShapeDirectlyBelow(i);
		if (!MFnDagNode(shape).findPlug("intermediateObject").asBool()){
			path = shape;
			return true;
		}
	}

	return false;
}

MObject Utils::findSkinCluster(MDagPath mesh)
{

	MStatus status;



	if (!Utils::saferExtendToShape(mesh))
		return MObject::kNullObj;

	MFnDagNode meshNode(mesh,&status);

	CHECK_STATUS("dag node not found from dag path in findSkinCluster",status);
	MPlug plug = meshNode.findPlug("inMesh");
	MItDependencyGraph it(plug,MFn::kInvalid,
                            MItDependencyGraph::kUpstream,
                            MItDependencyGraph::kDepthFirst,
                            MItDependencyGraph::kPlugLevel,
                            &status);
	CHECK_STATUS("iterate skin clusters",status);
	it.disablePruningOnFilter();

	for  (;!it.isDone();it.next()) {
		if (it.thisNode().hasFn(MFn::kSkinClusterFilter)){
			//cout << "found skin cluster on " << mesh.fullPathName() << ": " << it.thisPlug().info() << endl;
			return it.thisNode();
		}
	}


	return MObject();
}



void Utils::getSelectionFromString(const MString sel, MSelectionList &selection){
	MStringArray splitResult;
	sel.split('/',splitResult);
	for (unsigned i=0,count=splitResult.length();i<count;i++)
		selection.add(splitResult[i]);
}

bool Utils::findChildAttr(MObject &parentAttr,MString childName,MObject &childAttr){
	MFnCompoundAttribute parentFn(parentAttr);
	for (unsigned int i=0;i<parentFn.numChildren();i++){
		childAttr = parentFn.child(i);
		MFnAttribute childFn(childAttr);
		if (childFn.name()==childName)
			return true;
	}
	childAttr = MObject::kNullObj;

	return false;
}

bool Utils::findOutputMesh(MPlug &sourcePlug, MObject &shape){
	MStatus status;
	MItDependencyGraph it(sourcePlug,MFn::kInvalid,
							MItDependencyGraph::kDownstream,
                            MItDependencyGraph::kDepthFirst,
                            MItDependencyGraph::kPlugLevel,
                            &status);
	for  (;!it.isDone();it.next()) {
		if (it.thisNode().hasFn(MFn::kMesh)){
			shape = it.thisNode();
			return true;
		}
	}
	return false;
}

void Utils::enableDisplayColors(MObject &meshNode,const bool enable){
	MFnDependencyNode meshFn(meshNode);
	// TODO: could actually need to set those values to previous ones when colors are disabled again.
	meshFn.findPlug("displayColors").setBool(enable);
	meshFn.findPlug("displayColorChannel").setString(enable?"None":"Ambient+Diffuse");
}

bool Utils::shiftPressed(){
#ifdef WIN32
	return (GetAsyncKeyState(VK_SHIFT)&0x8000)!=0; 

#endif

	return false;
}

bool Utils::ctrlPressed(){
#ifdef WIN32
	return (GetAsyncKeyState(VK_CONTROL)&0x8000)!=0; 
#endif

	return false;
}

void mStringToDoubleArray(const MString &src, MDoubleArray &result) {
	MStringArray strings;
	src.split(',',strings);
	result.setLength(strings.length());
	for (unsigned int i=0;i<strings.length();i++){
		result.set(strings[i].asDouble(),i);
	}
}