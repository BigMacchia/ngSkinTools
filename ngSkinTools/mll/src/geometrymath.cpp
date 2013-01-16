#include "geometrymath.h"
#include "defines.h"

double GeometryMath::distanceToSegment(const MVector &segmentStart, const MVector &segmentEnd, const MVector &point,MVector *hitPoint){
	MVector ab = segmentEnd-segmentStart;
	MVector ap = point-segmentStart;

	double dotBoundsTest = ap*ab.normal();
	MVector projection = ab.normal()*dotBoundsTest;
	MVector magnitude = ap-projection;

	if (dotBoundsTest<=0) {
		// return distance start--point
		if (hitPoint)
			*hitPoint = segmentStart;
		return  projection.length()+magnitude.length();
		//return ap.length();
	}

	if (dotBoundsTest>=ab.length()){
		// return distance end--point
		if (hitPoint)
			*hitPoint = segmentEnd;
		return  projection.length()+magnitude.length();

		//MVector bp(point);
		//bp -= segmentEnd;
		//return bp.length();
	}

	if (hitPoint)
		*hitPoint = segmentStart+projection;

	return magnitude.length();
}
