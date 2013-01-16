#pragma once

#undef ERROR
#include <glog/logging.h>
#include <gtest/gtest.h>

#include <maya/MFileIO.h>
#include "../SkinLayerWeightList.h"
#include "../SkinLayer.h"
#include "../SkinLayerManager.h"
#include "../SkinLayerChanges.h"
#include "../ngSkinLayerCmd.h"


#include "mayaSupport.h"

class ManagerBasedTest: public ::testing::Test
{
public:
	ManagerBasedTest(){
		setupMayaLibrary();
	}

protected:
	InfluenceWeightsMap *map;
	SkinLayer * layer;
	SkinLayerManager manager;



	/**
	 * opens maya file and initializes manager, layer and map values
	 */
	virtual void openFile(MString fileName,MString meshShapeName, MString skinClusterName){
		TestManagerLocator::instance.clear();
		LOG(INFO)<<"Loading test file \""<<fileName<<"\"";
		MStatus status = MFileIO::open(fileName,NULL,true,MFileIO::kLoadDefault,true);
		
		ASSERT_EQ(status, MStatus::kSuccess);

		MSelectionList list;
		MObject mesh;
		MObject skinCluster;
		list.add(meshShapeName);
		list.add(skinClusterName);
		ASSERT_EQ(MStatus::kSuccess, list.getDependNode(0,mesh));
		ASSERT_EQ(MStatus::kSuccess, list.getDependNode(1,skinCluster));
		MDagPath path;
		list.getDagPath(0,path);

		TestManagerLocator::instance.addManager(path,manager);

		manager.setSkinCluster(skinCluster,mesh);
		addNewLayer("base weights");
		layer = manager.currentLayer;
		map = &layer->influenceWeightList;
	}

	void prepareSkinLayerChange(SkinLayerChanges::SkinLayerChange &change){
		change.manager = &manager;
	}

	void initSimpleMirrorData(){
		RuleDescriptionList ruleList;

		MatchNameRule rule;
		rule.addPrefix("L_");
		rule.addPrefix("R_");
		ruleList.addRule(&rule);
		manager.initSkinMirrorData(ruleList);
	}

	/**
	 * add new layer and set it current (simulate "add new layer" from UI)
	 */
	void addNewLayer(MString name){
		SkinLayerChanges::AddLayer ad;
		this->prepareSkinLayerChange(ad);
		ad.parent=manager.rootLayer;
		ad.newName=name;
		ad.execute();

		SkinLayerChanges::ChangeCurrentLayer cl(ad.layer);
		prepareSkinLayerChange(cl);
		cl.execute();

		int l=9;
		
		ASSERT_STREQ(manager.currentLayer->getName().asChar(l),ad.newName.asChar(l));
	}

	void deleteLayer(SkinLayer * layer){
		SkinLayerChanges::RemoveLayer rm;
		prepareSkinLayerChange(rm);
		rm.layer = layer;
		rm.execute();
	}

	void doMirror(bool mirrorWeights=true,bool mirrorMask=true,double mirrorWidth=0,bool positiveToNegative=false, bool guessMirrorSide=false){
		SkinLayerChanges::MirrorLayer ml(*manager.currentLayer);
		prepareSkinLayerChange(ml);
		ml.mirrorWeights=mirrorWeights;
		ml.mirrorMask = mirrorMask;
		ml.mirrorWidth=mirrorWidth;
		ml.positiveToNegative=positiveToNegative;
		ml.guessMirrorSide=guessMirrorSide;
		ml.execute();
	}

	/**
	 * paint using values from 
	 * SkinLayerPaintGlobals::brushIntensity
	 * and SkinLayerPaintGlobals::currentPaintMode
	 */
	void doFloodWeights(){
		SkinLayerChanges::FloodWeights fw(*manager.currentLayer);
		prepareSkinLayerChange(fw);
		fw.execute();
	}

	void doSetPaintTarget(int target){
		SkinLayerChanges::ChangeCurrentInfluence ci(target);
		prepareSkinLayerChange(ci);
		ci.execute();
	}

};


class ManagerBasedWeightListTests: public ManagerBasedTest
{
public:
	/**
	 * shorthand method to set influence 0, 1, 2 weights on vertex 0
	 */
	inline void setMapValues(const double v1,const double v2,const double v3){
		*map->getLogicalInfluence(0,0) = v1;
		*map->getLogicalInfluence(1,0) = v2;
		*map->getLogicalInfluence(2,0) = v3;
		map->recalcTransparency(0);
	}

	/**
	 * working on just three influences, test if expected values, passed as parameters,
	 * match influence weights on vertex 0, logical influences 0,1,2;
	 * transparency value is checked as well, expected to be a 1-weightSum
	 */
	inline void expectMapValues(const double v1,const double v2,const double v3){
		const double allowedError = 0.00000001;
		ASSERT_NEAR(v1,*map->getLogicalInfluence(0,0),allowedError);
		ASSERT_NEAR(v2,*map->getLogicalInfluence(1,0),allowedError);
		ASSERT_NEAR(v3,*map->getLogicalInfluence(2,0),allowedError);

		const double transparency = 1.0 - v1 - v2 - v3;
		const double actualTransparency = *map->getLogicalInfluence(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX,0);
		ASSERT_NEAR(transparency,actualTransparency,allowedError) << "expected transparency is " << transparency;

	}
};
