#pragma once

#include "SkinLayerWeightList.h"

class PaintStrokeInfo
{
private:
	SkinLayerWeightList lastStrokeWeights;
	double lastSideBias;
	const WeightTransferAssociation * currentAssociation;

	/**
	 * calculates the side in which the stroke was most effective
	 * positive means positive side, negative is negative side
	 */
	double calculateSide(const WeightTransferAssociation & transferAssociation) const;
public:
	PaintStrokeInfo();

	inline void setSize(unsigned int size){
		lastStrokeWeights.resize(size);
	}

	void startStroke(const WeightTransferAssociation &transferAssociation);
	void stopStroke();

	inline void setWeight(unsigned int vertex,const double weight){
		lastStrokeWeights.setWeight(vertex,weight);
	}

	inline double getLastSide() const{
		return this->lastSideBias;
	}




	~PaintStrokeInfo(void);
};
