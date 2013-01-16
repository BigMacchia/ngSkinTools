#include <vector>
#include <gtest/gtest.h>
#include <glog/logging.h>
#include <maya/MFileIO.h>
#include <maya/MGlobal.h>


#include "../maya.h"
#include "../MakeRigidWeights.h"
#include "../GeometryInfo.h"
#include "../utils.h"

using namespace std;


class MakeRigidWeightsTest: public ::testing::Test
{
protected:
	MakeRigidWeights engine;
public:
	void openFile(MString fileName){
		MStatus status = MFileIO::open(fileName,NULL,true,MFileIO::kLoadDefault,true);
		ASSERT_EQ(status, MStatus::kSuccess);
	}

	MakeRigidWeightsTest(void){
	}


	bool weightsEqual(const WeightedVertex &v1, const WeightedVertex &v2){
		double * w1 = v1.skinWeights;
		double * w2 = v2.skinWeights;
		for (unsigned int i=0;i<v1.geometry.influencesMask.size();i++, w1++, w2++){
			if (!isCloseTo(*w1, *w2)){
				return false;
			}
		}
		return true;
	}
};


TEST_F(MakeRigidWeightsTest,testSingleCluster){
	LOG(INFO) << "Starting MakeRigidWeightsTest,testSingleCluster test";
	openFile("test-resources/normalization.ma");

	// select non-contiguous selection
	MGlobal::clearSelectionList();
	MGlobal::selectByName("testMesh.vtx[0:1]");
	MGlobal::selectByName("testMesh.vtx[12:13]");
	MGlobal::getActiveSelectionList(engine.vertSelection);

	engine.isSingleClusterMode = true;
	engine.execute();


	ASSERT_EQ(4, engine.getVertList().size());

	WeightedVertex *firstVertex = engine.getVertList().front();

	for (std::vector<WeightedVertex *>::const_iterator it=engine.getVertList().begin();it!=engine.getVertList().end();it++){
		if (!weightsEqual(*firstVertex,**it)) {
				firstVertex->dumpCurrentWeights();
				(*it)->dumpCurrentWeights();
				FAIL()<<
					"vertices "<<
					firstVertex->vertNum<<
					" and "<< 
					(*it)->vertNum<<
					" are not equal";
		}

	}

	engine.getVertList();
}
