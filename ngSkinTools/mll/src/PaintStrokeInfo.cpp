#include <limits>
#include "PaintStrokeInfo.h"

#undef max
#undef min



PaintStrokeInfo::PaintStrokeInfo(void): lastSideBias(0)
{
}

PaintStrokeInfo::~PaintStrokeInfo(void)
{
}


double PaintStrokeInfo::calculateSide(const WeightTransferAssociation &transferAssociation) const {
	double result = 0;

	double positiveAverage=0,negativeAverage=0;
	unsigned int positiveSum=0,negativeSum=0;

	double const * w = lastStrokeWeights.getWeights();
	for (unsigned int i=0;i<lastStrokeWeights.getSize();i++,w++){
		if (*w!=0){
			if (transferAssociation.getVertexTransferInfo(i).isPositiveSide()){
				positiveSum++;
				positiveAverage += *w;
			}
			else {
				negativeSum++;
				negativeAverage += *w;
			}
		}
	}

	positiveAverage = positiveSum==0?0:positiveAverage/positiveSum;
	negativeAverage = negativeSum==0?0:negativeAverage/negativeSum;

	// exception
	if (positiveAverage==0 && negativeAverage==0){
		return 0;
	}

	// most common outcome
	if (positiveAverage==0 || negativeAverage==0){
		return positiveAverage-negativeAverage;
	}

	// both non-zero
	double ratio = std::max(positiveAverage,negativeAverage)/std::min(positiveAverage,negativeAverage);
	if (ratio<1.5)
		return 0;

	return positiveAverage-negativeAverage;
}

void PaintStrokeInfo::startStroke(const WeightTransferAssociation &transferAssociation){
	this->currentAssociation = &transferAssociation;

	double * w = lastStrokeWeights.getWeightPtr(0);
	for (unsigned int i=0;i<lastStrokeWeights.getSize();i++,w++){
		*w = 0;
	}
}

void PaintStrokeInfo::stopStroke(){
	
	if (currentAssociation->isInitialized()){
		double side = calculateSide(*currentAssociation);

		if (side!=0)
			this->lastSideBias = side;
	}

}

