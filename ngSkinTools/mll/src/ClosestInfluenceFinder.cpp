#include  <algorithm>
#include <maya/MFloatPointArray.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include <float.h>

#include "ClosestInfluenceFinder.h"
#include "maya.h"
#include "defines.h"
#include "geometrymath.h"
#include "utils.h"
#include "StatusException.h"

ClosestInfluenceFinder::ClosestInfluenceFinder(void):
accelParamsInitialized(false)
{
}

ClosestInfluenceFinder::~ClosestInfluenceFinder(void)
{
}

inline void getTranslationFromMatrixPlug(const MPlug &plug,const MMatrix *geomTransform,MVector &result){
	MFnMatrixData data(plug.asMObject());
	MMatrix m = data.matrix().inverse();

	// got to use MPoint here as vector does not work well with matrix operations
	
	result = MPoint(0,0,0,1)*m;//*geomTransform.inverse();
	if (geomTransform)
		result *= geomTransform->inverse();
}

bool ClosestInfluenceFinder::precalcInfluenceShape(const MDagPath &influence,const MMatrix *geomTransform){
	InfluenceSegment newSegment;
	newSegment.inflPath = influence;


	// get bind pre-matrix for this influence
	MFnDependencyNode skinClusterNode(*this->skinCluster);
	MPlug preMatrixList = skinClusterNode.findPlug("bindPreMatrix");
	// TODO: return false if influence is not present in skin cluster
	MPlug preMatrix1 = preMatrixList.elementByLogicalIndex(this->skinFn->indexForInfluenceObject(influence));
	getTranslationFromMatrixPlug(preMatrix1,geomTransform,newSegment.p1);
	newSegment.p2 = newSegment.p1;
	for (unsigned int i=0,count=influence.childCount();i<count;i++){
		MObject child = influence.child(i);
		if (child.hasFn(MFn::kTransform)){
			// found transform-compatible child; predict pre-bind coordinates now

			MDagPath path2(influence);
			path2.push(child);
			MStatus influenceFound;
			MPlug preMatrix2 = preMatrixList.elementByLogicalIndex(this->skinFn->indexForInfluenceObject(path2,&influenceFound));
			// child exists in influence list
			if (influenceFound==MStatus::kSuccess){
				getTranslationFromMatrixPlug(preMatrix2,geomTransform,newSegment.p2);
				break;
			}

			// TODO: predict child position when child is not present as influence in skin cluster;
			break;

			// child does not exist in influence list: use some more math
			MFnTransform childNode(child);
			//childNode.transformationMatrix
			MFnMatrixData(preMatrix1.asMObject()).matrix();

		}
	}

	this->segmentInfo.push_back(newSegment);
	return true;
}

bool ClosestInfluenceFinder::findClosestInfluence(MVector &pointPos,MDagPath &closestInfluence,const bool useIntersectionRanking){
	double shortestDistance = DBL_MAX;
	unsigned int minIntersections = UINT_MAX;

	// find closest influence and distance
	VECTOR_FOREACH(InfluenceSegment,this->segmentInfo,i){
		// get distance to joint

		MVector closestPoint;
		double currDist = GeometryMath::distanceToSegment(i->p1,i->p2,pointPos,&closestPoint);

		unsigned int numIntersections = 0;
		if (useIntersectionRanking) {
			// get number of intersections now
			MFloatPoint raySource(pointPos);
			MFloatVector rayDirection = closestPoint-pointPos;
			MFloatPointArray hits;
			MIntArray hitFaces;

			this->meshFn->allIntersections(raySource,rayDirection,NULL,NULL,false,MSpace::kWorld, 1.0,false,
				this->getAccelParams(),false, hits,NULL,&hitFaces,NULL,NULL,NULL);

			// reduce the number of intersections taken in mind
			// discard self-intersection and "touch-like " intersections
			numIntersections = hits.length();
			for (unsigned h=0;h<hits.length();h++){
				// TODO: this is a cheap and not too reliable way to discard self-intersections (at pointPos)
				if (isCloseToZero(raySource.distanceTo(hits[h]))){
					numIntersections--;
					continue;
				}

				//check normal x hit perpendicularity
				// if ray direction is perpendicular with face normal at hit point, we're only
				// touching the surface (rarely, this can happen)
				MVector polygonNormal;
				this->meshFn->getPolygonNormal(hitFaces[h],polygonNormal,MSpace::kWorld);
				if (isCloseToZero(polygonNormal.normal()*rayDirection)){
					numIntersections--;
					continue;
				}

			}
		}


		if (minIntersections>numIntersections || (minIntersections==numIntersections && shortestDistance>currDist)){
			minIntersections = numIntersections;
			shortestDistance = currDist;
			closestInfluence = i->inflPath;
		}
	}
	return true;
}

MMeshIsectAccelParams * ClosestInfluenceFinder::getAccelParams() {
	if (!this->accelParamsInitialized){
		this->accelParams = this->meshFn->autoUniformGridParams();
		this->accelParamsInitialized = true;
	}
	return &this->accelParams;
}


double ClosestInfluenceFinder::shortestDistanceToCloud(MDagPath &influence, MPointArray &pts){

	VECTOR_FOREACH(InfluenceSegment,this->segmentInfo,i){
		if (i->inflPath==influence){
			double result = DBL_MAX;
			for (unsigned int pt=0,count=pts.length();pt<count;pt++){

				MVector currPt(pts[pt]), hitPoint;
				GeometryMath::distanceToSegment(i->p1,i->p2,currPt,&hitPoint);
				hitPoint -= currPt;
				double currDist = hitPoint.length();
				if (currDist<result)
					result = currDist;
			}
			return result;
		}
	}
	return DBL_MAX;
}
