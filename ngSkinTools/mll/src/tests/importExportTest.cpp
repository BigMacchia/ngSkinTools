#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
using namespace std;

#include "../InfluenceMatchRules.h"
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"
#include "../StatusException.h"


inline void initDoubleArray(MDoubleArray &result,const unsigned int size){
	result.setLength(size);
	for (unsigned int i=0;i<result.length();i++){
		result.set(0.0,i);
	}
}

/**
 * compare two arrays, raise error if not equal
 */
inline void assertEquals(const MDoubleArray & array1,const MDoubleArray & array2){
	ASSERT_EQ(array1.length(),array2.length());
	for (unsigned int i=0;i<array1.length();i++){
		ASSERT_NEAR(array1[i],array2[i],0.000001);
	}
}

TEST_F(ManagerBasedTest,setInfluenceWeights){

	openFile("test-resources/merged-vertices.ma","pPlaneShape1","skinCluster1");

	const int influence = 0;
	addNewLayer("new layer");

	InfluenceWeightsMap & map =  manager.currentLayer->influenceWeightList;
	map.addInfluenceMapping(influence);

	// create new weights
	MDoubleArray weights;
	initDoubleArray(weights,map.getNumVerts());
	weights.set(0.6,0);
	weights.set(0.5,1);
	weights.set(0.4,2);


	// execute change
	SkinLayerChanges::SetLayerWeights w(*manager.currentLayer);
	ASSERT_EQ(w.getTarget(),SkinLayer::PAINT_TARGET_UNDEFINED);
	w.setNewWeights(weights);
	w.setTarget(influence);
	w.execute();

	// check that we're successful
	MDoubleArray newWeights;
	map.getLogicalInfluenceWeights(influence,newWeights);
	assertEquals(weights,newWeights);

	// repeat the check after multiple undos/redos
	for (int i=0;i<10;i++){
		w.undo();
		w.redo();
	}
	map.getLogicalInfluenceWeights(influence,newWeights);


	assertEquals(weights,newWeights);
}

TEST_F(ManagerBasedTest,setMaskWeights){
	openFile("test-resources/merged-vertices.ma","pPlaneShape1","skinCluster1");

	addNewLayer("new layer");
	SkinLayerWeightList &mask = manager.currentLayer->maskWeightList;
         
	// create new weights
	MDoubleArray weights;
	initDoubleArray(weights,manager.getMeshVertCount());
	weights.set(0.6,0);
	weights.set(0.5,1);
	weights.set(0.4,2);


	// execute change
	SkinLayerChanges::SetLayerWeights w(*manager.currentLayer);
	w.setNewWeights(weights);
	w.setTarget(SkinLayer::PAINT_TARGET_MASK);
	w.execute();

	// check that we're successful
	MDoubleArray newWeights;
	mask.getWeights(newWeights);
	assertEquals(weights,newWeights);

	// repeat the check after multiple undos/redos
	for (int i=0;i<10;i++){
		w.undo();
		w.redo();
	}
	mask.getWeights(newWeights);


	assertEquals(weights,newWeights);
}

TEST_F(ManagerBasedTest,setInvalidCount){
	openFile("test-resources/merged-vertices.ma","pPlaneShape1","skinCluster1");

	MDoubleArray weights;
	weights.setLength(2);
	weights.set(0.6,0);
	weights.set(0.5,1);


	SkinLayerChanges::SetLayerWeights w(*manager.currentLayer);
	w.setNewWeights(weights);
	w.setTarget(SkinLayer::PAINT_TARGET_MASK);
	ASSERT_THROW(w.execute(),StatusException);
}

TEST_F(ManagerBasedTest,setEmptyMask){
	openFile("test-resources/merged-vertices.ma","pPlaneShape1","skinCluster1");

	MDoubleArray weights;
	
	SkinLayerChanges::SetLayerWeights w(*manager.currentLayer);
	w.setNewWeights(weights);
	w.setTarget(SkinLayer::PAINT_TARGET_MASK);
	w.execute();
}

TEST_F(ManagerBasedTest,setEmptyInfluence){
	openFile("test-resources/merged-vertices.ma","pPlaneShape1","skinCluster1");

	MDoubleArray weights;
	
	SkinLayerChanges::SetLayerWeights w(*manager.currentLayer);
	w.setNewWeights(weights);
	w.setTarget(1);
	ASSERT_THROW(w.execute(),StatusException);
}

TEST_F(ManagerBasedTest,setInvalidIndex){
	openFile("test-resources/merged-vertices.ma","pPlaneShape1","skinCluster1");

	MDoubleArray weights;
	initDoubleArray(weights,manager.getMeshVertCount());
	
	SkinLayerChanges::SetLayerWeights w(*manager.currentLayer);
	w.setNewWeights(weights);
	w.setTarget(6666);
	ASSERT_THROW(w.execute(),StatusException);
}


TEST_F(ManagerBasedTest,mStringToDoubleArray){
	MDoubleArray result;
	mStringToDoubleArray("1.3, 1.5, 1.6",result);
	ASSERT_EQ(result.length(),3);
	ASSERT_NEAR(result[0],1.3,0.00001);
	ASSERT_NEAR(result[1],1.5,0.00001);
	ASSERT_NEAR(result[2],1.6,0.00001);
}
