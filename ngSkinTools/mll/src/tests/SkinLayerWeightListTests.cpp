#include <iostream>
#include "../SkinLayerWeightList.h"
#include "../SkinLayerManager.h"
#include <maya/MFileIO.h>
#include <maya/MDagPath.h>
#include "ManagerBasedTest.h"

#include "../SkinLayerChanges.h"

class SkinLayerWeightListTests: public ::testing::Test
{
public:
	static const double CALCULATION_ERROR;
};

const double SkinLayerWeightListTests::CALCULATION_ERROR = 0.000001;


TEST_F(SkinLayerWeightListTests,normalize){
	InfluenceWeightsMap map(true);
	map.resize(1,3,false);

	// add three influences, logical indexes slightly different than physical
	map.addInfluenceMapping(1);
	map.addInfluenceMapping(2);
	map.addInfluenceMapping(3);

	const unsigned int vertIndex = 0;
	
	*map.getLogicalInfluence(1,vertIndex) = 0.9;
	*map.getLogicalInfluence(2,vertIndex) = 0.05;
	*map.getLogicalInfluence(3,vertIndex) = 0.05;

	map.normalizeWeights(0,InfluenceWeightsMap::UNDEFINED_PHYSICAL_INFLUENCE,1.0,true);

	ASSERT_NEAR(0,*map.getLogicalInfluence(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX,vertIndex),CALCULATION_ERROR) << "transparency should be 0";
	ASSERT_NEAR(0.9,*map.getLogicalInfluence(1,vertIndex),CALCULATION_ERROR) << "no change expected";
	ASSERT_NEAR(0.05,*map.getLogicalInfluence(2,vertIndex),CALCULATION_ERROR);
	ASSERT_NEAR(0.05,*map.getLogicalInfluence(3,vertIndex),CALCULATION_ERROR);


	// skipped influences should zero out when non-skipped influence takes all available space
	*map.getLogicalInfluence(1,vertIndex) = 1.0;
	*map.getLogicalInfluence(2,vertIndex) = 1.0e-10;
	*map.getLogicalInfluence(3,vertIndex) = 1.0e-10;
	map.normalizeWeights(0,1,1.0,true);
	ASSERT_NEAR(1.0,*map.getLogicalInfluence(1,vertIndex),1.0e-11);
	ASSERT_NEAR(0,*map.getLogicalInfluence(2,vertIndex),1.0e-11);
	ASSERT_NEAR(0,*map.getLogicalInfluence(3,vertIndex),1.0e-11);
}

TEST_F(SkinLayerWeightListTests,normalizeIfNeeded){
	InfluenceWeightsMap map(true);
	map.resize(1,3,false);

	map.addInfluenceMapping(1);
	map.addInfluenceMapping(2);

	const unsigned int vertIndex = 0;
	
	// set both influences to 0.3 and make sure that stays!
	MDoubleArray weights;
	weights.setLength(1);
	weights.set(0.3,0);
		
	map.setInfluenceWeights(1,weights);
	map.setInfluenceWeights(2,weights);

	ASSERT_NEAR(0.3,*map.getLogicalInfluence(1,vertIndex),1.0e-11);
	ASSERT_NEAR(0.3,*map.getLogicalInfluence(2,vertIndex),1.0e-11);
}

TEST_F(SkinLayerWeightListTests,normalizeToRemainingSpace){
	InfluenceWeightsMap map(true);
	map.resize(1,3,false);

	map.addInfluenceMapping(1);
	map.addInfluenceMapping(2);

	const unsigned int vertIndex = 0;
	
	// set both influences to 0.3 and make sure that stays!
	MDoubleArray weights;
	weights.setLength(1);
	weights[0] = 0.7;
		
	// some transparency happens here
	map.setInfluenceWeights(1,weights);
	ASSERT_NEAR(0.3,*map.getLogicalInfluence(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX,vertIndex),1.0e-11);

	// transparency gets eaten away; influence 1 reduced to 0.3, as second influence is set to 0.7
	map.setInfluenceWeights(2,weights);
	ASSERT_NEAR(0.0,*map.getLogicalInfluence(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX,vertIndex),1.0e-11);
	ASSERT_NEAR(0.3,*map.getLogicalInfluence(1,vertIndex),1.0e-11);
	ASSERT_NEAR(0.7,*map.getLogicalInfluence(2,vertIndex),1.0e-11);
}

TEST_F(SkinLayerWeightListTests,applyInfluenceLimit){
	InfluenceWeightsMap map(false);
	map.resize(1,4,false);
	map.addInfluenceMapping(1);
	map.addInfluenceMapping(2);
	map.addInfluenceMapping(3);
	map.addInfluenceMapping(4);
	map.addInfluenceMapping(5);


	*map.getLogicalInfluence(1,0) = 0.05;
	*map.getLogicalInfluence(2,0) = 0.21;
	*map.getLogicalInfluence(3,0) = 0.5;
	*map.getLogicalInfluence(4,0) = 0.19;
	*map.getLogicalInfluence(5,0) = 0.05;

	map.limitNumberOfInfluences(0,0,2);

	ASSERT_NEAR(0,*map.getLogicalInfluence(1,0),1.0e-11);
	ASSERT_NEAR(0,*map.getLogicalInfluence(4,0),1.0e-11);
	ASSERT_NEAR(0,*map.getLogicalInfluence(5,0),1.0e-11);

	ASSERT_NEAR(1.0,*map.getLogicalInfluence(2,0)+*map.getLogicalInfluence(3,0),1.0e-11);

	ASSERT_NEAR(0.295,*map.getLogicalInfluence(2,0),0.001);
	ASSERT_NEAR(0.704,*map.getLogicalInfluence(3,0),0.001);
}

TEST_F(SkinLayerWeightListTests,applyInfluenceLimitWithMax1){
	InfluenceWeightsMap map(false);
	map.resize(1,4,false);
	map.addInfluenceMapping(1);
	map.addInfluenceMapping(2);
	map.addInfluenceMapping(3);


	*map.getLogicalInfluence(1,0) = 0.05;
	*map.getLogicalInfluence(2,0) = 0.21;
	*map.getLogicalInfluence(3,0) = 0.5;

	map.limitNumberOfInfluences(0,0,1);

	ASSERT_NEAR(0,*map.getLogicalInfluence(1,0),1.0e-11);
	ASSERT_NEAR(0,*map.getLogicalInfluence(2,0),1.0e-11);
	ASSERT_NEAR(1,*map.getLogicalInfluence(3,0),1.0e-11);
}

TEST_F(SkinLayerWeightListTests,applyInvalidInfluenceLimit){
	InfluenceWeightsMap map(false);
	map.resize(1,4,false);
	map.addInfluenceMapping(1);
	map.addInfluenceMapping(2);
	map.addInfluenceMapping(3);


	*map.getLogicalInfluence(1,0) = 0.05;
	*map.getLogicalInfluence(2,0) = 0.21;
	*map.getLogicalInfluence(3,0) = 0.5;

	// limit to more influences than there currently are
	map.limitNumberOfInfluences(0,0,10);

	ASSERT_NEAR(0.05,*map.getLogicalInfluence(1,0),1.0e-11);
	ASSERT_NEAR(0.21,*map.getLogicalInfluence(2,0),1.0e-11);
	ASSERT_NEAR(0.5,*map.getLogicalInfluence(3,0),1.0e-11);
}

TEST_F(SkinLayerWeightListTests,applyInfluenceLimitMultipleVerts){
	InfluenceWeightsMap map(false);
	map.resize(2,3,false);
	map.addInfluenceMapping(1);
	map.addInfluenceMapping(2);
	map.addInfluenceMapping(3);


	*map.getLogicalInfluence(1,0) = 0.1;
	*map.getLogicalInfluence(2,0) = 0.2;
	*map.getLogicalInfluence(3,0) = 0.4;

	*map.getLogicalInfluence(1,1) = 0.2;
	*map.getLogicalInfluence(2,1) = 0.4;
	*map.getLogicalInfluence(3,1) = 0.4;


	map.limitNumberOfInfluences(0,1,2);

	ASSERT_NEAR(0,*map.getLogicalInfluence(1,0),0.0001);
	ASSERT_NEAR(0.3333,*map.getLogicalInfluence(2,0),0.0001);
	ASSERT_NEAR(0.6666,*map.getLogicalInfluence(3,0),0.0001);

	ASSERT_NEAR(0,*map.getLogicalInfluence(1,1),0.001);
	ASSERT_NEAR(0.5,*map.getLogicalInfluence(2,1),0.001);
	ASSERT_NEAR(0.5,*map.getLogicalInfluence(3,1),0.001);
}


/**
 * test: distribute weight evenly to the remaining influences
 */
TEST_F(ManagerBasedWeightListTests,distributeWeightEvenly){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,distributeWeightEvenly test";

	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	setMapValues(0.9,0.05,0.05);
	layer->setCurrPaintTarget(0);
	manager.startPaintStroke();
	layer->setPaintValue(pmReplace,0.0,1.0,0,NULL);
	manager.endPaintStroke();
	expectMapValues(0, 0.5, 0.5);
}


/**
 * test: preserve transparency
 */
TEST_F(ManagerBasedWeightListTests,preserveTransparency){
	LOG(INFO) << "Starting ManagerBasedWeightListTests,preserveTransparency test";

	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	setMapValues(0,0.05,0.05);
	layer->setCurrPaintTarget(1);
	manager.startPaintStroke();
	layer->setPaintValue(pmReplace,0.0,1.0,0,NULL);
	manager.endPaintStroke();
	expectMapValues(0,0,0.1);
}

/**
 * test: preserve transparency
 */
TEST_F(ManagerBasedWeightListTests,setEnabled){
	openFile("test-resources/normalization.ma","testMeshShape","skinCluster1");

	SkinLayerChanges::ChangeEnabled e;
	e.manager = &manager;
	e.layer = layer;
	e.newEnabled = false;
	e.execute();

	ASSERT_EQ(layer->isEnabled(),false);
}