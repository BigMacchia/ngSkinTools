#pragma once
#include <vector>

using namespace std;

class LimitWeightsUtil
{
public:

	/**
	 * hard limit for influence count: if vertex has more than this amount
	 * of influences, they're ordered by weight and then least influential
	 * ones are removed
	 */
	unsigned int hardLimit;

	/**
	 * remove all influences that have their weight below given threshold
	 * (equivalent to "remove small influences")
	 */
	double dropThreshold;

	LimitWeightsUtil(void);
	~LimitWeightsUtil(void);

	void applyLimit(double *weights,const vector<bool> &influenceMask);
};
