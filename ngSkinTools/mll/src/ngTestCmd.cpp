#ifdef _DEBUG

#include "ngTestCmd.h"
#include "StatusException.h"
#include "ClusteredPointCloud.h"
#include "IPointCloud.h"
#include <time.h>
#include <iostream>
#include <sstream>
#include "utils.h"

#include <maya/MFnAttribute.h>
#include <maya/MFnCompoundAttribute.h>

using namespace std;

// initialize callback list
MCallbackIdArray ngTestCmd::callbacks;


void* ngTestCmd::creator()
{
	return new ngTestCmd();
}


ngTestCmd::ngTestCmd(void)
{
}

ngTestCmd::~ngTestCmd(void)
{
}



const char TEST_FLAG[] = "-nm";
const char TEST_FLAG_LN[] = "-longName";


MObject skinClusterWeightsAttribute;

void detectSkinClusterWeightsAttribute(MObject skinCluster){
	// detects skin cluster weights attribute (not plug) for quick comparision in the weights change event
	MFnCompoundAttribute weightList(MFnDependencyNode(skinCluster).attribute("weightList"));
	skinClusterWeightsAttribute = weightList.child(0);
}

void hookSkinClusterChanged(MNodeMessage::AttributeMessage msg, MPlug & plug, MPlug & otherPlug, void* customData ){
	if (MGlobal::isUndoing()||MGlobal::isRedoing())
		return;


	if (plug.attribute()!=skinClusterWeightsAttribute)
		return;

	ostringstream message;

	message << "skin cluster changed: "<< plug.info();
	MGlobal::displayInfo(message.str().c_str());
}

void ngTestCmd::testHookSkinClusterChange(MSelectionList &list){
	MItSelectionList selectionIterator(list);
	for (;!selectionIterator.isDone();selectionIterator.next()){
		MDagPath path;
		selectionIterator.getDagPath(path);
		MObject skinCluster = Utils::findSkinCluster(path);
		if (skinCluster.isNull())
			continue;

		detectSkinClusterWeightsAttribute(skinCluster);

		ngTestCmd::callbacks.append(MNodeMessage::addAttributeChangedCallback(skinCluster,hookSkinClusterChanged));
	}



}

MStatus ngTestCmd::doIt( const MArgList& args ) {


	try {

		// parse args
		MStatus status;
		MArgDatabase argData( syntax(), args, &status );
		CHECK_STATUS("initialize arg data",status);



		MSelectionList sel,tempSelection;
		status = argData.getObjects(tempSelection);
		CHECK_STATUS("arg data: get objects",status);
		if (tempSelection.length()>0) {
			/*
			status = sel.merge(tempSelection,MSelectionList::kMergeNormal);
			CHECK_STATUS("merge selection list");

			testHookSkinClusterChange(sel);

			double radius = 3;
			if (argData.isFlagSet("r"))
				radius = argData.flagArgumentDouble("r",0,&status);

			this->pointCloudTest(sel,radius);
			*/
		}
		else {
			DEBUG_COUT_ENDL("nothing is selected?");
		}




	}
	catch (StatusException &e){
		MGlobal::displayError(e.getStatus().errorString());
		return e.getStatus();
	}

	return this->redoIt();
}



void ngTestCmd::pointCloudTest(MSelectionList selList,double radius){
	DEBUG_COUT_ENDL("point cloud test!");

	// iterate over whole dag
	MItSelectionList selectionIterator(selList);
	for (;!selectionIterator.isDone();selectionIterator.next()){

		// get current selection path
		MDagPath path;	MObject compSelection;
		selectionIterator.getDagPath(path,compSelection);

		// we require selection to have vertex component selection
		if (!selectionIterator.hasComponents() || !compSelection.hasFn(MFn::kMeshVertComponent)){
			MGlobal::displayWarning(path.fullPathName()+" doesn't have vertex selection specified, skipping");
			continue;
		}
		MItMeshVertex testVertexIterator(path,compSelection);
		DEBUG_COUT_ENDL("testing point cloud building on:" << path.fullPathName());


		MFnMesh mesh(path);
		MPointArray points;
		mesh.getPoints(points);



		clock_t execTime = clock();
		IPointCloud * cloud = new ClusteredPointCloud(radius);
		cloud->setCapacity(points.length());
		for (unsigned int i=0;i<points.length();i++)
			cloud->addPoint(reinterpret_cast<void *>(i),points[i]);

		// create selection around first selected vertex
		MFnSingleIndexedComponent newSelection;
		MObject components = newSelection.create(MFn::kMeshVertComponent);
		for (;!testVertexIterator.isDone();testVertexIterator.next())
			for (IPointCloud::IPointIterator * it=cloud->getNearbyVerts(radius,points[testVertexIterator.index()]);it->next();){
				newSelection.addElement(static_cast<int>(reinterpret_cast<size_t>(it->getData())));
			}
		MGlobal::select(path,components,MGlobal::kReplaceList);

		execTime = clock()-execTime;
		DEBUG_COUT_ENDL("successfully tested "<<points.length()<<" points, time (ms): " << execTime*1000.0/CLOCKS_PER_SEC);

		delete cloud;
		DEBUG_COUT_ENDL("successfully deleted point cloud!");

	}
}



MStatus ngTestCmd::redoIt() {
	return MS::kSuccess;
}

MStatus ngTestCmd::undoIt() {
	return MS::kSuccess;
}

bool ngTestCmd::isUndoable() const {
	return true;
}

bool ngTestCmd::hasSyntax(){
	return true;
}

MSyntax ngTestCmd::newSyntax(){
	MSyntax syntax;
	syntax.useSelectionAsDefault(true);
	syntax.setObjectType(MSyntax::kSelectionList,1);

	syntax.addFlag("r", "-stepSize", MSyntax::kDouble);

	syntax.enableQuery( false );
	syntax.enableEdit( false );
	return syntax;
}

#endif
