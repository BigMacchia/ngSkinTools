#include <algorithm>
#include "defines.h"
#include "InflListerBase.h"

InflListerBase::InflListerBase(void)
{
}

InflListerBase::~InflListerBase(void)
{
}


void InflListerBase::normalizeTotalWeights(){
	// get sum of all weights
	double sum = 0;
	VECTOR_FOREACH(InfluenceInfo,this->inflList,i)
		sum += i->totalWeight;

	// normalize each weight to sum of 1.0
	VECTOR_FOREACH(InfluenceInfo,this->inflList,i)
		i->totalWeight /= sum;

}

void InflListerBase::getResult(MStringArray &result,bool longNames){

	VECTOR_FOREACH(InfluenceInfo,this->inflList,i){
		// add influence name
		result.append(longNames?i->influence.fullPathName():i->influence.partialPathName());

		// add weight as float
		MString weight;
		weight.set(i->totalWeight);
		result.append(weight);
	}

}

void InflListerBase::sortByTotalWeight(bool biggestFirst){
	CompareByTotalWeight comp(biggestFirst);
	std::sort(this->inflList.begin(),this->inflList.end(),comp);
}
