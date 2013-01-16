#pragma once
#include <vector>
#include "maya.h"
using namespace std;

/**
 * representation of an influence as line segment (joints with one child)
 */
class InfluenceSegment{
public:
	MDagPath inflPath;
	MVector p1;
	MVector p2;
};


/**
 * this engine is responsible for providing utilities of finding
 * closest influence in a skin cluster
 */
class ClosestInfluenceFinder
{
private:
	MFnMesh *meshFn;
	MFnSkinCluster *skinFn;
	MObject *skinCluster;

	
	// mesh intersection acceleration
	MMeshIsectAccelParams accelParams;
	bool accelParamsInitialized;
public:
	ClosestInfluenceFinder(void);
	~ClosestInfluenceFinder(void);

	MMeshIsectAccelParams * getAccelParams();

	vector<InfluenceSegment> segmentInfo;

	void setMeshInfo(MObject * skinCluster,MFnMesh *meshFn,MFnSkinCluster *skinFn) {
		this->meshFn = meshFn;
		this->skinCluster = skinCluster;
		this->skinFn = skinFn;
	}

	/**
	 * stores influence "geometry" data - data that is used to calculate closest distance to joint
	 * this method should be called for all influences that should be considered
	 * as search result
	 * 
	 * if geom transform is given, influence positions are multipled to inverse of this
	 */
	bool precalcInfluenceShape(const MDagPath &influence,const MMatrix *geomTransform);

	bool findClosestInfluence(MVector &pointPos,MDagPath &closestInfluence,const bool useIntersectionRanking);

	/**
	 * returns closest distance from influence to point cloud
	 */
	double shortestDistanceToCloud(MDagPath &influence,MPointArray &pts);

};
