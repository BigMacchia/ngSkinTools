#pragma once

#include <vector>
#include <set>
#include "LimitWeightsUtil.h"
#include "maya.h"
#include "WeightsModifierEngine.h"
#include "timer.h"

class RelaxEngine: public WeightsModifierEngine
{
private:


	/**
	 * initializes tension for all editable verts
	 */
	void initVertTension();

public:


	/**
	 * number of steps to repeat relax procedure. will be set by
	 * command by reading user input.
	 */
	int numSteps;


	/**
	 * intensity of one relax step. will be set by
	 * command by reading user input.
	 */
	double stepSize;


	LimitWeightsUtil weightsLimiter;
	bool weightLimitEnabled;


	RelaxEngine(void);
	~RelaxEngine(void);

	/**
	 * executes relax procedure, reading neccessary data and calculating
	 * new skin weights to be set. this procedure does not do any modification
	 * to the scene; to store results, call #writeSkinWeights afterwards.
	 */
	void execute();


	virtual WeightedVertex * createVertexInfoInstance(GeometryInfo &parent);

	#ifdef _SHOW_TIMERS
	static debug::Timer timerRelax;
	static debug::Timer timerRelaxInner;
	#endif
};

