#pragma once
#include "AssignWeightsEngineBase.h"
#include "WeightedVertex.h"


class RigidWeightsVertex: public WeightedVertex {
private:
	/**
	 * continues clustering those neighbours that fall into the same cluster
	 */
	void continueClusterNeighbours(RigidWeightsVertex ** currentTail);

	/**
	 * simplified version of clustering - just puts every 
	 * vertex into one cluster
	 */
	void createSingleCluster();

public:
	/**
	 * starts recursive clustering; does nothing, 
	 * if vertex already belongs to a cluster
	 */
	void solveCluster();

	/**
	 * vertex owning cluster "HEAD". if it's NULL, vertex
	 * does not belong to any cluster
	 */
	RigidWeightsVertex * clusterRoot;

	/**
	 * all clusters are stored in a uni-directional chain.
	 * this pointeir points to next vertex in cluster (in no particular order)
	 */
	RigidWeightsVertex * clusterNext;
	
	RigidWeightsVertex(GeometryInfo &parent):
		WeightedVertex(parent),
		clusterRoot(NULL),
		clusterNext(NULL)
	{
	}
};


class MakeRigidWeights :
	public AssignWeightsEngineBase
{
public:
	MakeRigidWeights(void);
	virtual ~MakeRigidWeights(void);

	WeightedVertex * createVertexInfoInstance(GeometryInfo &parent){
		return new RigidWeightsVertex(parent);
	}

	bool isSingleClusterMode;

	void execute();

};
