#pragma once

#include <map>
#include "maya.h"
#include "WeightedVertex.h"
#include "AssignWeightsEngineBase.h"
#include "ClosestInfluenceFinder.h"
#include "GeometryInfo.h"

class VertexClosestJointInfo: public WeightedVertex {
private:
public:
	VertexClosestJointInfo(GeometryInfo &parent):	WeightedVertex(parent)
	{
	}

	void calcShortestDistance();
};

class ClosestJointGeometryInfo: public GeometryInfo {
public:
	ClosestInfluenceFinder influenceFinder;

	ClosestJointGeometryInfo(WeightsModifierEngine * engine): GeometryInfo(engine){};
	virtual void init();
};

class WeightsByClosestJoint: public AssignWeightsEngineBase
{

	
public:
	/**
	 * when this list is not empty, only those influences will be used
	 * as possible choice when searching for influences
	 */
	MSelectionList includedInfluences;

	/**
	 * returns true, if influence should be used for calculations
	 * (it's allowed to use by include/exclude rules)
	 */
	bool useInfluence(MDagPath &influence);

	virtual WeightedVertex * createVertexInfoInstance(GeometryInfo &parent) {
		return new  VertexClosestJointInfo(parent);
	}
	virtual GeometryInfo * createGeometryInfoInstance() {
		return new ClosestJointGeometryInfo(this);
	}

	/**
	 * When set to true, uses additional ranking by intersections
	 * when finding a closest joint.
	 */
	bool useIntersectionRanking;
	
	WeightsByClosestJoint(void);
	~WeightsByClosestJoint(void);

	void execute();
};
