#include "LimitWeightsUtil.h"
#include "defines.h"

LimitWeightsUtil::LimitWeightsUtil(void)
{
}

LimitWeightsUtil::~LimitWeightsUtil(void)
{
}

void LimitWeightsUtil::applyLimit(double *weights, const std::vector<bool> &influenceMask){
	/*
	1. make a list of N=limit records, defining N biggest weights;
	2. foreach weight that is less than smallest of those weights, set zero weight.
	*/

	// weights will be ordered from smallest to biggest
	double *maxWeights = new double[this->hardLimit];
	for (unsigned int i=0;i<this->hardLimit;i++) {
		maxWeights[i] = 0;
	}

	// find maximums first
	double * currWeights = weights;
	for (size_t i=0,count=influenceMask.size();i<count;i++,currWeights++){
		if (!influenceMask[i])
			continue;

		// smaller than smallest maximum, skip the rest of checking
		if (*currWeights<*maxWeights)
			continue;

		// insert new maximum, shifting all smaller maximums out of array
		double * currMax = maxWeights;
		for (double currIndex = 0;currIndex<this->hardLimit && *currWeights>*currMax;currMax++,currIndex++) {
			if (currIndex>0)
				currMax[-1] = currMax[0];
			currMax[0] = *currWeights;
		}
	}

	// drop all weights that are smaller than smallest maximum
	currWeights = weights;
	for (size_t i=0,count=influenceMask.size();i<count;i++,currWeights++){
		if (!influenceMask[i])
			continue;

		if (*currWeights<*maxWeights && *currWeights>0){
			*currWeights = 0.0;
		}
	}

	delete [] maxWeights;

}
