#include <sstream>

#include "defines.h"
#include "InflListerByWeights.h"
#include "utils.h"
#include "StatusException.h"
#include "GeometryInfo.h"
#include "WeightedVertex.h"
#include "WeightsModifierEngine.h"

void InflListerByWeights::execute(){
	this->initVertList();
	this->initSkinWeights();


	// from all loaded influences, create a list
	VECTOR_FOREACH(MDagPath,this->influences,i){
		InfluenceInfo info;
		info.influence = *i;
		this->inflList.push_back(info);
	}
	
	
	// sum all weights
	VECTOR_FOREACH(WeightedVertex *,this->vertList,vert){
		
		double *currWeight = (*vert)->skinWeights;
		GeometryInfo &geomInfo = (*vert)->geometry;
		for (size_t inflIndex=0,count=geomInfo.numVertWeights();inflIndex<count;inflIndex++,currWeight++){
			if (geomInfo.influencesMask[inflIndex])
				this->inflList[inflIndex].totalWeight += *currWeight;
		}
	}


	this->normalizeTotalWeights();
	this->sortByTotalWeight(true);
}




