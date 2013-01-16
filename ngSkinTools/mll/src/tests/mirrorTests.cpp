#include "../SkinLayerWeightList.h"
#include "../SkinLayerManager.h"
#include <maya/MFileIO.h>
#include <maya/MDagPath.h>
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"

/**
 * bug reproduce/fix: mirror empty layer raises assertion error
 */
TEST_F(ManagerBasedWeightListTests,mirrorEmptyLayer){
	LOG(INFO)<<"Starting ManagerBasedWeightListTests,mirrorEmptyLayer test";
	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	initSimpleMirrorData();

	addNewLayer("second empty layer");

	doMirror();
}


/**
 * bug reproduce/fix: mirror empty layer raises assertion error
 */
TEST_F(ManagerBasedWeightListTests,guessMirrorSide){
	openFile("test-resources/simplemirror.ma","testMeshShape","skinCluster1");

	initSimpleMirrorData();

	const int R_Joint1 = 5;
	const int L_Joint1 = 1;
	const unsigned int leftVert = 12;
	const unsigned int rightVert = 0;

	SkinLayerPaintGlobals::brushIntensity = 1.0;
	SkinLayerPaintGlobals::currentPaintMode = pmReplace;
	
	doSetPaintTarget(R_Joint1);
	manager.startPaintStroke();
	manager.currentLayer->setPaintValue(pmReplace,.66,1.0,rightVert,NULL);
	manager.endPaintStroke();
	ASSERT_TRUE(manager.lastStrokeInfo.getLastSide()<0);
	doMirror(true,true,0.1,true, true);
	ASSERT_NEAR(*manager.rootLayer->finalWeightList.getLogicalInfluence(L_Joint1,leftVert),.66,0.0001);

	// paint on the other side, but mirror with same options
	doSetPaintTarget(L_Joint1);
	manager.startPaintStroke();
	manager.currentLayer->setPaintValue(pmReplace,.9,1.0,leftVert,NULL);
	manager.endPaintStroke();
	ASSERT_TRUE(manager.lastStrokeInfo.getLastSide()>0);
	doMirror(true,true,0.1,true, true);
	ASSERT_NEAR(*manager.rootLayer->finalWeightList.getLogicalInfluence(R_Joint1,rightVert),.9,0.0001);


	// switch sides again, paint on one side, then fill, then mirror
	manager.startPaintStroke();
	doSetPaintTarget(R_Joint1);
	manager.currentLayer->setPaintValue(pmReplace,1.0,1.0,rightVert,NULL);
	manager.endPaintStroke();

	ASSERT_TRUE(manager.lastStrokeInfo.getLastSide()<0);

	manager.startPaintStroke();
	doSetPaintTarget(R_Joint1);
	manager.currentLayer->setPaintValue(pmReplace,.8,1.0,rightVert,NULL);
	doSetPaintTarget(L_Joint1);
	manager.currentLayer->setPaintValue(pmReplace,.7,1.0,leftVert,NULL);
	manager.endPaintStroke();

	ASSERT_TRUE(manager.lastStrokeInfo.getLastSide()<0);

}