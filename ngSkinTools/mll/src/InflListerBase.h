#pragma once
#include "WeightsModifierEngine.h"


/**
 * one entry about influence, stored in influence lister
 */
class InfluenceInfo
{
public:

	InfluenceInfo():totalWeight(0){}

	double totalWeight;
	MDagPath influence;
};


/**
 * compare utility to sort InfluenceInfo list by totalWeight (biggest first)
 */
class CompareByTotalWeight
{
private:
	bool biggestFirst;
public:
	CompareByTotalWeight(bool bf):biggestFirst(bf){
	}

	bool operator()( const InfluenceInfo &info1, const InfluenceInfo &info2 ) const {
		return biggestFirst?info1.totalWeight>info2.totalWeight:info1.totalWeight<info2.totalWeight;
	}
};

class InflListerBase :
	public WeightsModifierEngine
{
protected:

	/**
	 * result list
	 */
	std::vector<InfluenceInfo> inflList;

	void normalizeTotalWeights();
	void sortByTotalWeight(bool biggestFirst);
public:
	InflListerBase(void);
	virtual ~InflListerBase(void);


	/**
	 * puts calculation results to string array, 
	 * ready to be delivered to command result
	 */
	void getResult(MStringArray &result,bool longNames);

};
