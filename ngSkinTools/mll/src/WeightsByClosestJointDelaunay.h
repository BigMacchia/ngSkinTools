#pragma once
#include "AssignWeightsEngineBase.h"
#include "maya.h"

class WeightsByClosestJointDelaunay: public AssignWeightsEngineBase
{
private:

	// workaround for tetgen bug: adding identical points cause
	// tetrahedralization to crash
	MPointArray addedPoints;

	bool isPointAdded(const MPoint & pt);
	void addPoint(MPoint & pt);

public:
	WeightsByClosestJointDelaunay(void);
	~WeightsByClosestJointDelaunay(void);

	void execute();

};
