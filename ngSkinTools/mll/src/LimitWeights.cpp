#include <float.h>
#include "LimitWeights.h"
#include "LimitWeightsUtil.h"
#include "ProgressWindow.h"
#include "GeometryInfo.h"
#include "WeightedVertex.h"
#include "defines.h"

LimitWeights::LimitWeights()
{
	this->numInflLimit = 4;
}

LimitWeights::~LimitWeights()
{
}

void LimitWeights::execute(){
	this->initVertList();
	this->initVertSoftSelection();
	this->initSkinWeights();

	ProgressWindow progress("Limit Weights",static_cast<int>(this->vertList.size()));
	
	VECTOR_FOREACH(WeightedVertex *,this->vertList,i){
		static_cast<LimitWeightsVertex *>(*i)->applyWeightLimit(this->numInflLimit);
		progress.add();
		if (progress.isCanceled())
			return;
	}


	this->applyEffectIntensity();
	this->applySelectionWeights();
	this->finished();
}

void LimitWeightsVertex::applyWeightLimit(const unsigned int limit)
{
	this->initializeSecondaryWeightsBuffers();

	LimitWeightsUtil limiter;
	limiter.hardLimit = limit;
	limiter.applyLimit(this->skinWeights,this->geometry.influencesMask);
	


	this->normalizeWeights();
}
