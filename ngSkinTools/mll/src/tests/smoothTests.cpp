#include "../SkinLayerWeightList.h"
#include "../SkinLayerManager.h"
#include <maya/MFileIO.h>
#include <maya/MDagPath.h>
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"


/**
 NGST-93:Modifying vertex count makes flood operation crash maya
 */
TEST_F(ManagerBasedWeightListTests,ngst93){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,ngst93 test";
	openFile("test-resources/merged-vertices.ma","pPlaneShape1","skinCluster1");

	doSetPaintTarget(0);
	SkinLayerPaintGlobals::brushIntensity = 1.0;
	SkinLayerPaintGlobals::currentPaintMode = pmSmooth;

	doFloodWeights();


}

/**
 NGST-104: Maya 2013 does not save or load layers correctly
 */
TEST_F(ManagerBasedWeightListTests,ngst104){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,ngst104 test";
	openFile("test-resources/saved layers.mb","pPlaneShape1","skinCluster1");

}