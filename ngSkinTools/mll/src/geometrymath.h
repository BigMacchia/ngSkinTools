#pragma once
#include "maya.h"


namespace GeometryMath {
	/**
	 * returns distance of "point" to line segment defined by segmentStart and segmentEnd
	 * distance returned is either to one of ends of segment, or shortest distance anywhere inside segment, whichever is shorter
	 */
	double distanceToSegment(const MVector &segmentStart, const MVector &segmentEnd, const MVector &point,MVector *resultPoint=NULL);

}

