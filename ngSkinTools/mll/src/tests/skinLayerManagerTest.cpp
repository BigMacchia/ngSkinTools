#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
using namespace std;

#include "../InfluenceMatchRules.h"
#include "ManagerBasedTest.h"
#include "../SkinLayerChanges.h"
#include "../StatusException.h"
#include "../ngSkinLayerCmd.h"


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