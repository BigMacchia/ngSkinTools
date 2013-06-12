#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
using namespace std;

#include "../InfluenceMatchRules.h"
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"
#include "../StatusException.h"
#include "../ngSkinLayerCmd.h"
#include "../SkinLayerData.h"
#include "../utils.h"


TEST_F(ManagerBasedTest,addRemoveManualMirrorInfluences){
	openFile("test-resources/simplemirror.ma","testMeshShape","skinCluster1");

	MStringArray result;

	ngSkinLayerCmd cmd;
	cmd.setLayerManager(manager);
	cmd.queryManualMirrorInfluenceAssociations(result);
	ASSERT_EQ(result.length(),0);


	const MString rightJoint("x_axis|root|R_joint1");
	const MString leftJoint("x_axis|root|L_joint1");

	manager.addMirrorInfluenceAssociation(rightJoint,leftJoint);
	manager.addMirrorInfluenceAssociation(leftJoint,rightJoint);

	cmd.queryManualMirrorInfluenceAssociations(result);
	ASSERT_EQ(result.length(),4);

	// result list comes arranged in any order, as manager stores them in a map; it so happens that left joint comes first 
	// in this case as it has lower index.
	ASSERT_EQ(result[0],leftJoint);
	ASSERT_EQ(result[1],rightJoint);
	ASSERT_EQ(result[2],rightJoint);
	ASSERT_EQ(result[3],leftJoint);
}

TEST(SkinLayerManagerTest,detachedManagerLoadSave){
	setupMayaLibrary();

	SkinLayerManager manager1;
	SkinLayer * layer = manager1.createLayer();
	layer->setName("whatever name");
	layer->setParent(manager1.rootLayer);
	MDoubleArray weights;
	mStringToDoubleArray("0.2,0.3,0.5",weights);
	layer->influenceWeightList.addInfluenceMapping(3,0);
	layer->influenceWeightList.setLogicalInfluenceWeights(0,weights);

	std::stringstream io;
	SkinLayerData::saveManager(manager1,io);


	SkinLayerManager manager2;
	SkinLayerData::loadManager(manager2,io,SKIN_LAYER_DATA_VERSIONS::CURR);
	ASSERT_EQ(manager1.rootLayer->children.size(), manager2.rootLayer->children.size());

	ASSERT_EQ(MString("whatever name"), manager2.rootLayer->children[0]->getName());
	MDoubleArray loadedWeights;
	manager2.rootLayer->children[0]->influenceWeightList.getLogicalInfluenceWeights(0,loadedWeights);
	assertEquals(weights,loadedWeights);
}
