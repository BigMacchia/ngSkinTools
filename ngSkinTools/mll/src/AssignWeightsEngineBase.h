#pragma once
#include "WeightsModifierEngine.h"
#include "WeightedVertex.h"


class AssignWeightsEngineBase: public WeightsModifierEngine
{
	double intensity;
public:


	/**
	 * defines intensity amount of this weights engine. default = 1.0
	 */
	inline double getIntensity() const{
		return this->intensity;
	}

	void setIntensity(const double intensity);

	void applyEffectIntensity();

	AssignWeightsEngineBase();
};
