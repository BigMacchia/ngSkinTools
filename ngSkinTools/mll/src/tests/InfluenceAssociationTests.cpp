#include <gtest/gtest.h>
#include <glog/logging.h>

#include <iostream>
using namespace std;

#include "../InfluenceMatchRules.h"
#include "ManagerBasedTest.h"
#include "../WeightTransferAssociation.h"



class InfluenceAssociationTests: public ::testing::Test
{
protected:
	InfluenceTransferInfo *inflLeft;
	InfluenceTransferInfo *inflRight;
	InfluenceTransferInfoVec influences;
	inline InfluenceTransferInfo *addInfluence(const MString name, const MString longName,
		const unsigned int index, const double x, const double y, const double z)
	{

		InfluenceTransferInfo * info = new InfluenceTransferInfo();
		info->setName(name, longName);
		info->setLogicalIndex(index);
		info->setPivot(MVector(x,y,z));
		return addInfluence(info);
	}

	inline InfluenceTransferInfo *addInfluence(InfluenceTransferInfo * const info){
		influences.push_back(info);
		return info;
	}

public:
	InfluenceAssociationTests(void){
		inflLeft = this->addInfluence("L_influence","root|L_influence",0,10.0,10.0,10.0);
		inflRight = this->addInfluence("R_influence","root|R_influence",1,-10.0,10.0,10.0);
	}

};


TEST_F(InfluenceAssociationTests,ruleList){

	RuleDescriptionList ruleList;
	ruleList.isMirrorMode = true;

	MatchNameRule nameRule;
	ruleList.addRule(&nameRule);
	nameRule.addPrefix("L_");
	nameRule.addPrefix("R_");

	ruleList.resolveAssociations(influences,influences);

	ASSERT_EQ(inflLeft->getDestination(),inflRight) << "R_influence should be destination for L_influence";
	ASSERT_EQ(inflRight->getDestination(),inflLeft) << "L_influence should be destination for R_influence";
}

TEST_F(InfluenceAssociationTests,NameRuleUsage){
	RuleDescriptionList ruleList;
	ruleList.isMirrorMode = true;

	MatchNameRule nameRule;
	ruleList.addRule(&nameRule);
	nameRule.addPrefix("L_");
	nameRule.addPrefix("R_");

	ASSERT_EQ(nameRule.getPrefixes().length(), 2);
	nameRule.initMatching(*inflLeft);
	ASSERT_FALSE(nameRule.isMatched())<<"initial state should be false";
	ASSERT_EQ(NULL,nameRule.getMatch())<<"initial match should be null";

	nameRule.testMatch(*inflRight);
	ASSERT_TRUE(nameRule.isMatched())<<"match should have been found";
}

TEST_F(InfluenceAssociationTests,MatchDistanceRule){
	RuleDescriptionList ruleList;
	ruleList.isMirrorMode = true;
	

	MatchDistanceRule distanceRule;
	ruleList.addRule(&distanceRule);

	// add some imperfection to symmetry, and make that imperfection an
	// allowable error in rule
	MVector difference(0.1,0.1,0.1);
	inflLeft->setPivot(inflLeft->getPivot()+difference);
	// set threshold to just a little higher than distance between two points
	distanceRule.setThreshold(difference.length()+SMALL_NUMBER_LAMBDA);

	

	distanceRule.initMatching(*inflLeft);
	ASSERT_FALSE(distanceRule.isMatched())<<"initial state should be false";
	ASSERT_EQ(NULL,distanceRule.getMatch())<<"initial match should be null";

	distanceRule.testMatch(*inflRight);
	ASSERT_TRUE(distanceRule.isMatched())<<"match should have been found";


	distanceRule.initMatching(*inflLeft);
	difference.x = -difference.x;
	InfluenceTransferInfo &info3 = *addInfluence(new InfluenceTransferInfo(*inflRight));
	InfluenceTransferInfo &info4 = *addInfluence(new InfluenceTransferInfo(*inflRight));
	info3.setPivot(inflRight->getPivot()+difference);
	info4.setPivot(inflRight->getPivot()+difference*2);

	distanceRule.testMatch(*inflRight);
	// info3 is the best match, as it has the same offset as inflLeft
	distanceRule.testMatch(info3);
	distanceRule.testMatch(info4);

	ASSERT_TRUE(distanceRule.isMatched())<<"match should have been found";
	ASSERT_EQ(distanceRule.getMatch(),&info3)<<"best match is info3";
}

TEST_F(InfluenceAssociationTests,DistanceRuleOnSymmetryAxis){
	RuleDescriptionList ruleList;
	ruleList.isMirrorMode = true;

	MatchDistanceRule distanceRule;
	ruleList.addRule(&distanceRule);


	InfluenceTransferInfo *joint1 = addInfluence("joint","joint",5,0,1,1);
	InfluenceTransferInfo *joint2 = addInfluence("jointTwist","jointTwist",6,0,1,1);

	ruleList.resolveAssociations(influences, influences);
	ASSERT_EQ(joint1->getDestination(),joint1);

}

TEST_F(InfluenceAssociationTests,SampleRigDumpTest){
	RuleDescriptionList ruleList;
	ruleList.isMirrorMode = true;
	ruleList.setMirrorAxis(SkinToolsGlobal::X);

	MatchNameRule nameRule;
	MatchDistanceRule distanceRule;

	nameRule.addPrefix("L_");
	nameRule.addPrefix("R_");
	nameRule.addPrefix("Lf");
	nameRule.addPrefix("Rt");

	distanceRule.setThreshold(0.001);
	
	ruleList.addRule(&nameRule);
	ruleList.addRule(&distanceRule);

	addInfluence("tailJoint1", "|insectRig|joints|tailJoint1", 0, 0, 2.92386, 0.220526);
	InfluenceTransferInfo * tail2 = addInfluence("tailJoint2", "|insectRig|joints|tailJoint1|tailJoint2", 1, 8.95019e-017, 2.95956, -0.584844);
	addInfluence("tailJoint3", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3", 2, 1.79024e-016, 2.98856, -1.39048);
	addInfluence("tailJoint4", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4", 3, 2.68562e-016, 3.00913, -2.19638);
	addInfluence("tailJoint5", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5", 4, 3.58107e-016, 3.02001, -3.00246);
	addInfluence("tailJoint6", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6", 5, 4.47648e-016, 3.02229, -3.80862);
	addInfluence("tailJoint7", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7", 6, 5.37182e-016, 3.02024, -4.61478);
	addInfluence("tailJoint8", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8", 7, 6.26715e-016, 3.01694, -5.42093);
	addInfluence("tailJoint9", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9", 8, 7.16249e-016, 3.01459, -6.22709);
	addInfluence("tailJoint10", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10", 9, 8.05788e-016, 3.01546, -7.03325);
	addInfluence("tailJoint11", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11", 10, 8.95332e-016, 3.02134, -7.83939);
	addInfluence("tailJoint12", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12", 11, 9.84878e-016, 3.03125, -8.64549);
	addInfluence("tailJoint13", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13", 12, 1.07442e-015, 3.04319, -9.45156);
	addInfluence("tailJoint14", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13|tailJoint14", 13, 1.16397e-015, 3.05543, -10.2576);
	addInfluence("tailJoint15", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13|tailJoint14|tailJoint15", 14, 1.25351e-015, 3.0666, -11.0637);
	addInfluence("tailJoint16", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13|tailJoint14|tailJoint15|tailJoint16", 15, 1.34306e-015, 3.07458, -11.8698);
	addInfluence("tailJoint17", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13|tailJoint14|tailJoint15|tailJoint16|tailJoint17", 16, 1.4326e-015, 3.07714, -12.676);
	addInfluence("tailJoint18", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13|tailJoint14|tailJoint15|tailJoint16|tailJoint17|tailJoint18", 17, 1.52213e-015, 3.07277, -13.4821);
	addInfluence("tailJoint19", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13|tailJoint14|tailJoint15|tailJoint16|tailJoint17|tailJoint18|tailJoint19", 18, 1.61165e-015, 3.06041, -14.2883);
	addInfluence("tailJoint20", "|insectRig|joints|tailJoint1|tailJoint2|tailJoint3|tailJoint4|tailJoint5|tailJoint6|tailJoint7|tailJoint8|tailJoint9|tailJoint10|tailJoint11|tailJoint12|tailJoint13|tailJoint14|tailJoint15|tailJoint16|tailJoint17|tailJoint18|tailJoint19|tailJoint20", 19, 1.70112e-015, 3.03934, -15.0941);
	addInfluence("neckJoint1", "|insectRig|joints|neckJoint1", 20, 0, 2.91148, 0.269002);
	addInfluence("neckJoint2", "|insectRig|joints|neckJoint1|neckJoint2", 21, 0, 2.92454, 0.82052);
	addInfluence("neckJoint3", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3", 22, 5.32321e-017, 2.94367, 1.37187);
	addInfluence("neckJoint4", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4", 23, 2.21619e-016, 2.97522, 1.92263);
	addInfluence("neckJoint5", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5", 24, 5.58536e-016, 3.02573, 2.47196);
	addInfluence("neckJoint6", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6", 25, 1.10919e-015, 3.09985, 3.0186);
	addInfluence("neckJoint7", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7", 26, 1.88918e-015, 3.19891, 3.56126);
	addInfluence("neckJoint8", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8", 27, 2.89195e-015, 3.32317, 4.09871);
	addInfluence("neckJoint9", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8|neckJoint9", 28, 4.13335e-015, 3.47292, 4.62962);
	addInfluence("neckJoint10", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8|neckJoint9|neckJoint10", 29, 5.60277e-015, 3.64853, 5.15254);
	addInfluence("neckJoint11", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8|neckJoint9|neckJoint10|neckJoint11", 30, 7.31765e-015, 3.85072, 5.66578);
	addInfluence("neckJoint12", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8|neckJoint9|neckJoint10|neckJoint11|neckJoint12", 31, 9.26292e-015, 4.07829, 6.16829);
	addInfluence("neckJoint13", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8|neckJoint9|neckJoint10|neckJoint11|neckJoint12|neckJoint13", 32, 1.14059e-014, 4.32714, 6.66061);
	addInfluence("neckJoint14", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8|neckJoint9|neckJoint10|neckJoint11|neckJoint12|neckJoint13|neckJoint14", 33, 1.3695e-014, 4.59253, 7.14425);
	addInfluence("neckJoint15", "|insectRig|joints|neckJoint1|neckJoint2|neckJoint3|neckJoint4|neckJoint5|neckJoint6|neckJoint7|neckJoint8|neckJoint9|neckJoint10|neckJoint11|neckJoint12|neckJoint13|neckJoint14|neckJoint15", 34, 1.60996e-014, 4.87031, 7.62088);
	addInfluence("headJoint", "|insectRig|joints|headJoint", 35, 0, 5.04856, 7.6975);
	addInfluence("tailJoint1twist", "|insectRig|joints|tailJoint1twist", 36, 0, 2.92386, 0.220526);
	InfluenceTransferInfo * tailTwist2 = addInfluence("tailJoint2twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist", 37, 8.95019e-017, 2.95956, -0.584844);
	addInfluence("tailJoint3twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist", 38, 1.79024e-016, 2.98856, -1.39048);
	addInfluence("tailJoint4twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist", 39, 2.68562e-016, 3.00913, -2.19638);
	addInfluence("tailJoint5twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist", 40, 3.58107e-016, 3.02001, -3.00246);
	addInfluence("tailJoint6twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist", 41, 4.47648e-016, 3.02229, -3.80862);
	addInfluence("tailJoint7twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist", 42, 5.37182e-016, 3.02024, -4.61478);
	addInfluence("tailJoint8twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist", 43, 6.26715e-016, 3.01694, -5.42093);
	addInfluence("tailJoint9twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist", 44, 7.16249e-016, 3.01459, -6.22709);
	addInfluence("tailJoint10twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist", 45, 8.05788e-016, 3.01546, -7.03325);
	addInfluence("tailJoint11twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist", 46, 8.95332e-016, 3.02134, -7.83939);
	addInfluence("tailJoint12twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist", 47, 9.84878e-016, 3.03125, -8.64549);
	addInfluence("tailJoint13twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist", 48, 1.07442e-015, 3.04319, -9.45156);
	addInfluence("tailJoint14twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist|tailJoint14twist", 49, 1.16397e-015, 3.05543, -10.2576);
	addInfluence("tailJoint15twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist|tailJoint14twist|tailJoint15twist", 50, 1.25351e-015, 3.0666, -11.0637);
	addInfluence("tailJoint16twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist|tailJoint14twist|tailJoint15twist|tailJoint16twist", 51, 1.34306e-015, 3.07458, -11.8698);
	addInfluence("tailJoint17twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist|tailJoint14twist|tailJoint15twist|tailJoint16twist|tailJoint17twist", 52, 1.4326e-015, 3.07714, -12.676);
	addInfluence("tailJoint18twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist|tailJoint14twist|tailJoint15twist|tailJoint16twist|tailJoint17twist|tailJoint18twist", 53, 1.52213e-015, 3.07277, -13.4821);
	addInfluence("tailJoint19twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist|tailJoint14twist|tailJoint15twist|tailJoint16twist|tailJoint17twist|tailJoint18twist|tailJoint19twist", 54, 1.61165e-015, 3.06041, -14.2883);
	addInfluence("tailJoint20twist", "|insectRig|joints|tailJoint1twist|tailJoint2twist|tailJoint3twist|tailJoint4twist|tailJoint5twist|tailJoint6twist|tailJoint7twist|tailJoint8twist|tailJoint9twist|tailJoint10twist|tailJoint11twist|tailJoint12twist|tailJoint13twist|tailJoint14twist|tailJoint15twist|tailJoint16twist|tailJoint17twist|tailJoint18twist|tailJoint19twist|tailJoint20twist", 55, 1.70112e-015, 3.03934, -15.0941);
	addInfluence("neckJoint1twist", "|insectRig|joints|neckJoint1twist", 56, 0, 2.91148, 0.269002);
	addInfluence("neckJoint2twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist", 57, 0, 2.92454, 0.82052);
	addInfluence("neckJoint3twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist", 58, 5.32321e-017, 2.94367, 1.37187);
	addInfluence("neckJoint4twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist", 59, 2.21619e-016, 2.97522, 1.92263);
	addInfluence("neckJoint5twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist", 60, 5.58536e-016, 3.02573, 2.47196);
	addInfluence("neckJoint6twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist", 61, 1.10919e-015, 3.09985, 3.0186);
	addInfluence("neckJoint7twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist", 62, 1.88918e-015, 3.19891, 3.56126);
	addInfluence("neckJoint8twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist", 63, 2.89195e-015, 3.32317, 4.09871);
	addInfluence("neckJoint9twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist|neckJoint9twist", 64, 4.13335e-015, 3.47292, 4.62962);
	addInfluence("neckJoint10twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist|neckJoint9twist|neckJoint10twist", 65, 5.60277e-015, 3.64853, 5.15254);
	addInfluence("neckJoint11twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist|neckJoint9twist|neckJoint10twist|neckJoint11twist", 66, 7.31765e-015, 3.85072, 5.66578);
	addInfluence("neckJoint12twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist|neckJoint9twist|neckJoint10twist|neckJoint11twist|neckJoint12twist", 67, 9.26292e-015, 4.07829, 6.16829);
	addInfluence("neckJoint13twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist|neckJoint9twist|neckJoint10twist|neckJoint11twist|neckJoint12twist|neckJoint13twist", 68, 1.14059e-014, 4.32714, 6.66061);
	addInfluence("neckJoint14twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist|neckJoint9twist|neckJoint10twist|neckJoint11twist|neckJoint12twist|neckJoint13twist|neckJoint14twist", 69, 1.3695e-014, 4.59253, 7.14425);
	addInfluence("neckJoint15twist", "|insectRig|joints|neckJoint1twist|neckJoint2twist|neckJoint3twist|neckJoint4twist|neckJoint5twist|neckJoint6twist|neckJoint7twist|neckJoint8twist|neckJoint9twist|neckJoint10twist|neckJoint11twist|neckJoint12twist|neckJoint13twist|neckJoint14twist|neckJoint15twist", 70, 1.60996e-014, 4.87031, 7.62088);
	addInfluence("tailEnd", "|insectRig|joints|tailEnd", 71, 0, 3.39564, -13.5762);
	addInfluence("tail_tip_2", "|insectRig|joints|tailEnd|tail_tip_2", 72, -2.38595e-017, 3.14493, -14.4624);
	addInfluence("L_FrontLeg_Clavicle", "|insectRig|joints|front_leg_root|L_FrontLeg_Clavicle", 73, 0.235611, 4.2751, 7.54684);
	addInfluence("L_FrontLegShoulder", "|insectRig|joints|front_leg_root|L_FrontLeg_Clavicle|L_FrontLegShoulder", 74, 0.750662, 4.20923, 7.54684);
	addInfluence("L_FrontLegForearm", "|insectRig|joints|front_leg_root|L_FrontLeg_Clavicle|L_FrontLegShoulder|L_FrontLegForearm", 75, 1.14448, 2.9377, 7.28308);
	addInfluence("R_FrontLeg_Clavicle", "|insectRig|joints|front_leg_root|R_FrontLeg_Clavicle", 76, -0.235611, 4.2751, 7.54684);
	addInfluence("R_FrontLegShoulder", "|insectRig|joints|front_leg_root|R_FrontLeg_Clavicle|R_FrontLegShoulder", 77, -0.750662, 4.20923, 7.54684);
	addInfluence("R_FrontLegForearm", "|insectRig|joints|front_leg_root|R_FrontLeg_Clavicle|R_FrontLegShoulder|R_FrontLegForearm", 78, -1.14448, 2.9377, 7.28308);
	addInfluence("R_FrontLegWrist", "|insectRig|joints|front_leg_root|R_FrontLeg_Clavicle|R_FrontLegShoulder|R_FrontLegForearm|R_FrontLegWrist", 79, -0.962055, 4.02068, 8.5306);

	ruleList.resolveAssociations(influences,influences);

	ASSERT_EQ(tail2->getDestination(),tail2) << "tail2 should resolve to itself";
	ASSERT_EQ(tailTwist2->getDestination(),tailTwist2) << "tail2twist should resolve to itself";
}

TEST_F(ManagerBasedTest,testManualOverrides){
	openFile("test-resources/simplemirror.ma","testMeshYShape","skinClusterY");
	RuleDescriptionList ruleList;
	ruleList.isMirrorMode = true;
	ruleList.setMirrorAxis(SkinToolsGlobal::Y);

	MatchManualOverrideRule manualRule;
	manualRule.addOverride(1,5);

	MatchNameRule nameRule;
	MatchDistanceRule distanceRule;

	nameRule.addPrefix("L_");
	nameRule.addPrefix("R_");
	distanceRule.setThreshold(0.001);

	ruleList.addRule(&manualRule);
	ruleList.addRule(&nameRule);
	ruleList.addRule(&distanceRule);
	manager.initSkinMirrorData(ruleList);

	const InfluenceTransferInfoVec * inflAssociations = &manager.mirrorData.getInfluencesList();
	ASSERT_EQ(inflAssociations->findByShortName("L_joint1")->getLogicalIndex(),1);
	ASSERT_EQ(inflAssociations->findByShortName("R_joint2")->getLogicalIndex(),5);
	ASSERT_EQ(inflAssociations->findByShortName("L_joint1")->getDestination(),inflAssociations->findByShortName("R_joint2"));
}


class XYZAxisTester: public ManagerBasedTest {
protected:
	RuleDescriptionList ruleList;
	MatchDistanceRule distanceRule;
	const InfluenceTransferInfoVec * inflAssociations;

	virtual void openFile(MString fileName,MString meshShapeName, MString skinClusterName){
		ManagerBasedTest::openFile(fileName,meshShapeName,skinClusterName);
		manager.initSkinMirrorData(ruleList);

		inflAssociations = &manager.mirrorData.getInfluencesList();
	}

public:
	XYZAxisTester(){
		ruleList.isMirrorMode = true;
		ruleList.setMirrorAxis(SkinToolsGlobal::X);

		distanceRule.setThreshold(0.0001);

		ruleList.addRule(&distanceRule);
	}

};


TEST_F(XYZAxisTester,testXAxisAssociation){
	openFile("test-resources/simplemirror.ma","testMeshShape","skinCluster1");
	ASSERT_EQ(inflAssociations->findByShortName("root")->getDestination(),inflAssociations->findByShortName("root"));
	ASSERT_EQ(inflAssociations->findByShortName("rootTwist")->getDestination(),inflAssociations->findByShortName("rootTwist"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint1")->getDestination(),inflAssociations->findByShortName("R_joint1"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint2")->getDestination(),inflAssociations->findByShortName("R_joint2"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint3")->getDestination(),inflAssociations->findByShortName("R_joint3"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint4")->getDestination(),inflAssociations->findByShortName("R_joint4"));
}



void assertHasWeight(const VertexTransferInfo &info,unsigned int sourceVertexIndex, double expectedWeight){
	for (int i=0;i<2;i++){
		if (sourceVertexIndex==info.sourceVertex[i]){
			ASSERT_TRUE(isCloseTo(expectedWeight,info.weights[i]));
			return;
		}
	}
	FAIL();
}

TEST_F(XYZAxisTester,testYAxisAssociation){
	ruleList.setMirrorAxis(SkinToolsGlobal::Y);
	openFile("test-resources/simplemirror.ma","testMeshYShape","skinClusterY");
	ASSERT_EQ(inflAssociations->findByShortName("root")->getDestination(),inflAssociations->findByShortName("root"));
	ASSERT_EQ(inflAssociations->findByShortName("rootTwist")->getDestination(),inflAssociations->findByShortName("rootTwist"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint1")->getDestination(),inflAssociations->findByShortName("R_joint1"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint2")->getDestination(),inflAssociations->findByShortName("R_joint2"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint3")->getDestination(),inflAssociations->findByShortName("R_joint3"));


	// test some mirror pairs
	assertHasWeight(manager.mirrorData.vertexTransfer.getVertexTransferInfo(9),0,1.0);
	assertHasWeight(manager.mirrorData.vertexTransfer.getVertexTransferInfo(10),19,1.0);



	// select R_joint1
	const int inflR_Joint1 = 4;
	const int inflL_Joint1 = 1;
	layer->setCurrPaintTarget(inflL_Joint1);
	// paint 1.0 or positive side, and 0.0 on respective vertices on negative side
	manager.startPaintStroke();
	layer->setPaintValue(pmReplace,1.0,1.0,0,NULL);
	layer->setPaintValue(pmReplace,1.0,1.0,10,NULL);
	layer->setPaintValue(pmReplace,0.0,1.0,9,NULL);
	layer->setPaintValue(pmReplace,0.0,1.0,19,NULL);
	manager.endPaintStroke();


	// test for correct transfer
	manager.startPaintStroke();
	layer->setPaintValue(pmReplace,0.0,1.0,9,NULL);
	layer->setPaintValue(pmReplace,0.0,1.0,19,NULL);
	manager.endPaintStroke();
	layer->mirrorWeights(true,0,true,true,NULL,NULL);
	ASSERT_TRUE(isCloseTo(*layer->influenceWeightList.getLogicalInfluence(inflR_Joint1,9), 1.0));
	ASSERT_TRUE(isCloseTo(*layer->influenceWeightList.getLogicalInfluence(inflR_Joint1,19), 1.0));


	// soft mirror with some mirror width
	manager.startPaintStroke();
	layer->setPaintValue(pmReplace,0.0,1.0,9,NULL);
	layer->setPaintValue(pmReplace,0.0,1.0,19,NULL);
	manager.endPaintStroke();
	layer->mirrorWeights(true,0.001,true,true,NULL,NULL);
	ASSERT_TRUE(isCloseTo(*layer->influenceWeightList.getLogicalInfluence(inflR_Joint1,9), 1.0));
	ASSERT_TRUE(isCloseTo(*layer->influenceWeightList.getLogicalInfluence(inflR_Joint1,19), 1.0));


}
TEST_F(XYZAxisTester,testZAxisAssociation){
	ruleList.setMirrorAxis(SkinToolsGlobal::Z);
	openFile("test-resources/simplemirror.ma","testMeshZShape","skinClusterZ");
	ASSERT_EQ(inflAssociations->findByShortName("root")->getDestination(),inflAssociations->findByShortName("root"));
	ASSERT_EQ(inflAssociations->findByShortName("rootTwist")->getDestination(),inflAssociations->findByShortName("rootTwist"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint1")->getDestination(),inflAssociations->findByShortName("R_joint1"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint2")->getDestination(),inflAssociations->findByShortName("R_joint2"));
	ASSERT_EQ(inflAssociations->findByShortName("L_joint3")->getDestination(),inflAssociations->findByShortName("R_joint3"));
}
