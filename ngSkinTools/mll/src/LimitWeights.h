#pragma once
#include "AssignWeightsEngineBase.h"
#include "WeightedVertex.h"
#include "GeometryInfo.h"


class LimitWeightsVertex: public WeightedVertex {
private:
public:
	LimitWeightsVertex(GeometryInfo &parent):	WeightedVertex(parent)
	{
	}

	void applyWeightLimit(const unsigned int limit);
};

class LimitWeights:
	public AssignWeightsEngineBase
{

	
public:
	unsigned int numInflLimit;

	LimitWeights(void);
	virtual ~LimitWeights(void);
	void execute();

	virtual WeightedVertex * createVertexInfoInstance(GeometryInfo &parent) {
		return new  LimitWeightsVertex(parent);
	}

};
