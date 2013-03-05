#include "../SkinLayerWeightList.h"
#include "../SkinLayerManager.h"
#include <maya/MFileIO.h>
#include <maya/MDagPath.h>
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"
#include "../ngLayerColorDisplayNode.h"

#include "../SkinLayerWeightList.h"
#include "../SkinLayerManager.h"
#include <maya/MFileIO.h>
#include <maya/MDagPath.h>
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"
#include "../ngLayerColorDisplayNode.h"
#include "../StatusException.h"

#include "../SkinLayerPaintContextCmds.h"

/**
 * bug reproduce:
 * NGST-77 painting mask, but not smoothing it does not get propagated to layers
 */
TEST_F(ManagerBasedWeightListTests,ngst77){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,ngst77 test";
	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	doSetPaintTarget(0);
	SkinLayerPaintGlobals::brushIntensity = 1.0;
	SkinLayerPaintGlobals::currentPaintMode = pmReplace;

	doFloodWeights();

	addNewLayer("second layer");

	doSetPaintTarget(1);
	doFloodWeights();

	doSetPaintTarget(SkinLayer::PAINT_TARGET_MASK);

	WeightsChange oldWeights;
	manager.currentLayer->setPaintValue(pmReplace,1.0,1.0,0,&oldWeights);

	// first vertex should be assigned to influence 1, others to infl 0
	EXPECT_NEAR(*manager.rootLayer->finalWeightList.getLogicalInfluence(1,0),1.0,0.0001);
	EXPECT_NEAR(*manager.rootLayer->finalWeightList.getLogicalInfluence(0,1),1.0,0.0001);
	EXPECT_NEAR(*manager.rootLayer->finalWeightList.getLogicalInfluence(0,2),1.0,0.0001);
	EXPECT_NEAR(*manager.rootLayer->finalWeightList.getLogicalInfluence(0,3),1.0,0.0001);
}

/**
 * bug reproduce:
 * new layer, fill mask with zero, start painting with 1.0 -> produces crash
 */
TEST_F(ManagerBasedWeightListTests,newMaskPaintZeroCrash){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,newMaskPaintZeroCrash test";
	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	addNewLayer("second layer");

	doSetPaintTarget(SkinLayer::PAINT_TARGET_MASK);
	SkinLayerPaintGlobals::brushIntensity = 0.0;
	SkinLayerPaintGlobals::currentPaintMode = pmReplace;

	doFloodWeights();
	SkinLayerPaintGlobals::brushIntensity = 1.0;
	manager.startPaintStroke();
	manager.currentLayer->setPaintValue(pmReplace,1.0,1.0,0,NULL);
	manager.endPaintStroke();


	ngLayerColorDisplayNode displayNode;
	displayNode.displayCurrentWeights(*manager.currentLayer);

}

/**
 * bug reproduce:
 * all influences are zeroed out, but due to transparency being non-zero, mask is painted as white
 */
TEST_F(ManagerBasedWeightListTests,doNotPaintTransaprency){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,newMaskPaintZeroCrash test";
	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	addNewLayer("second layer");



	doSetPaintTarget(0);
	SkinLayerPaintGlobals::brushIntensity = 1.0;
	SkinLayerPaintGlobals::currentPaintMode = pmReplace;
	manager.startPaintStroke();
	manager.currentLayer->setPaintValue(pmReplace,1.0,1.0,0,NULL);
	manager.endPaintStroke();

	SkinLayerPaintGlobals::brushIntensity = 0.0;
	doFloodWeights();

	doSetPaintTarget(SkinLayer::PAINT_TARGET_MASK);
	SkinLayerPaintGlobals::brushIntensity = 1.0;
	SkinLayerPaintGlobals::currentPaintMode = pmReplace;
	doFloodWeights();

	ngLayerColorDisplayNode displayNode;
	displayNode.displayCurrentWeights(*manager.currentLayer);

	float (* colors)[4] = new float [displayNode.getCurrColors().length()][4];
	displayNode.getCurrColors().get(colors);
	for (unsigned int i=0;i<displayNode.getCurrColors().length();i++){
		LOG(INFO) << "vertex " <<i;
		ASSERT_NEAR(colors[i][0],1.0,0.0001);
		ASSERT_NEAR(colors[i][1],0.0,0.0001);
		ASSERT_NEAR(colors[i][2],0.0,0.0001);
	}

	delete [] colors;

}

/**
 * bug reproduce:
 * crash when having two meshes with the same name
 */
TEST_F(ManagerBasedWeightListTests,sameNameCrashTest){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,newMaskPaintZeroCrash test";
	openFile("test-resources/genericSkinnedMesh.mb","mesh|pPlane1","skinCluster1");

	LayerPaintCmds::InitializeCmd initialize;
	const int id = initialize.doIt("pPlaneShape1");
	ASSERT_EQ(id,LayerPaintCmds::LayerPaintStrokeInfo::NO_MANAGER) << "should return NO_MANAGER with duplicate shape IDs";

	LayerPaintCmds::FinalizeCmd finalize;
	ASSERT_THROW(finalize.doIt(id),StatusException);

}
