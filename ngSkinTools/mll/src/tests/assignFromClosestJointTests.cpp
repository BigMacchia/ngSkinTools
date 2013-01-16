#include "../SkinLayerWeightList.h"
#include "../SkinLayerManager.h"
#include <maya/MFileIO.h>
#include <maya/MDagPath.h>
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"
#include "../WeightsByClosestJoint.h"


class AssignFromClosestJointTest: public ManagerBasedWeightListTests {
};

/**
 * NGST-82 - assign weights from closest joint + flood smooth results in non-normalized weight values.
 */
TEST_F(AssignFromClosestJointTest,smoothAfterAssignWeights){
	LOG(INFO)<<"Starting smoothAfterAssignWeights test";

	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	SkinLayer * layer = manager.currentLayer;
	addNewLayer("second empty layer");
	
	deleteLayer(layer);


	// 1. assign from closest joint
	WeightsByClosestJoint engine;

	MGlobal::clearSelectionList();
	MGlobal::selectByName("testMesh");
	MGlobal::getActiveSelectionList(engine.vertSelection);

	engine.execute();
	engine.writeSkinWeights();
	InfluenceWeightsMap & wl = manager.currentLayer->influenceWeightList;
	EXPECT_NEAR(*wl.getLogicalInfluence(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX,0),0.0,0.0001);
	EXPECT_NEAR(wl.vertexWeightSum(0),1.0,0.0001);

	// 2. flood smooth
	SkinLayerPaintGlobals::brushIntensity = 1.0;
	SkinLayerPaintGlobals::currentPaintMode = pmReplace;
	doFloodWeights();
	EXPECT_NEAR(*wl.getLogicalInfluence(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX,0),0.0,0.0001);
	EXPECT_NEAR(wl.vertexWeightSum(0),1.0,0.0001);


}
