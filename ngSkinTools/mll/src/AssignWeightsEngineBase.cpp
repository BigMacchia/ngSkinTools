#include "AssignWeightsEngineBase.h"
#include "defines.h"

void AssignWeightsEngineBase::setIntensity(const double intensity){
	if (intensity>1.0){
		this->intensity = 1.0;
		return;
	}
	if (intensity<0.0){
		this->intensity = 0.0;
		return;
	}

	this->intensity = intensity;
}

AssignWeightsEngineBase::AssignWeightsEngineBase(){
	this->intensity = 1.0;
}

void AssignWeightsEngineBase::applyEffectIntensity(){
	if (this->intensity<1.0) {
		VECTOR_FOREACH(WeightedVertex *,this->vertList,i){
			(*i)->mixInOriginalWeights(this->getIntensity());
		}
	}
}
