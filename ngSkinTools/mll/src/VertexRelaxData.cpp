#include <sstream>
#include "defines.h"
#include "VertexRelaxData.h"
#include "GeometryInfo.h"
#include "RelaxEngine.h"
#include "assert.h"


VertexRelaxData::VertexRelaxData(GeometryInfo &parent):	WeightedVertex(parent)
{
}

void VertexRelaxData::initTension(){
	this->initNeighbours();
	if (this->weldedTo)
		return; // neighbour tension calculation will be handled by master 

	// currently "weight" in neighbours represents distance to it
	double totalDistance = 0;
	for (WeightedNeigbourMap::iterator i=neighbours.begin();i!=neighbours.end();i++){
		totalDistance += i->second;
	}

	// turn lengths into coeficients (totalLen/len: the smaller the distance, the bigger the weight)
	double totalWeights = 0;
	for (WeightedNeigbourMap::iterator i=neighbours.begin();i!=neighbours.end();i++){
		if (!isCloseToZero(i->second)){
			i->second = totalDistance/i->second;
			totalWeights += i->second;
		}
	}



	// normalize weights to sum of 1.0
	for (WeightedNeigbourMap::iterator i=neighbours.begin();i!=neighbours.end();i++){
		i->second /= totalWeights;
	}

}

void VertexRelaxData::performRelaxStep(const double stepSize) {
	// formula: 
	// 1. get vector: sum all neighbours,
	// 2. substract self weights vector;
	// 3. add fraction of the result to self weights
	// 4. normalize to sum=1.0 ?
	// allocate some lazy-initialized members 
	this->initializeSecondaryWeightsBuffers();


	// checking if we've got any free weight to manipulate on;
	// we can safely check for equality as we set that earlier in 
	// calculation if it's really close to zero
	// this way relax step can't do anything about this vertex, all weights are locked
	assert(this->totalFreeWeight>=0);
	if (this->totalFreeWeight==0){
		return;
	}
	

	// get the list of neighbours we're using
	WeightedNeigbourMap & neighboursToUse = this->getNeighbours();

	double sumNewWeights = 0;
	double * nextWeight = this->nextSkinWeights; // pointer for new calculated weight
	double * currWeight = this->skinWeights; // pointer to current weights, used for calculation
	
	std::vector<bool> &mask =this->geometry.influencesMask;
	std::vector<bool> &locked =this->geometry.inflLocked;
	for (size_t i=0,count=this->geometry.numVertWeights();i<count;i++,nextWeight++,currWeight++){
		if (!mask[i]){
			// this influence is not used by vertex's skin cluster
			// we're maintaining zeroes in unused weights already
			//*nextWeight = 0;
			_TIMING(RelaxEngine::timerRelaxInner.stop());
			continue;
		}

		
		if (locked[i]){
			// this weight should not be edited
			continue;
		}

		// calculate weighted sum of neighbours
		double neighbourSum = 0;
		for (WeightedNeigbourMap::iterator n=neighboursToUse.begin();n!=neighboursToUse.end();n++){
			// TODO: alter the weight from a neighbour according to bug #20
			WeightedVertex &currNeighbour = *n->first;
			if (&currNeighbour.geometry==&this->geometry){
				// for vertices from the same geometry: use simplified algorithm
				neighbourSum += currNeighbour.skinWeights[i]*n->second;
			}
			else {
				if (currNeighbour.skinWeights[i]>0 && currNeighbour.geometry.numVertWeights()>i && currNeighbour.geometry.influencesMask[i])
					neighbourSum += currNeighbour.skinWeights[i]*n->second*this->getCommonWeightMultiplier(currNeighbour);
			}
		}
		
		// blend some of this neighbour sum to current weight
		// formula: current weight + ((weight difference to neighbours)*step size)
		*nextWeight = *currWeight+(neighbourSum-(*currWeight))*stepSize;
		sumNewWeights +=  *nextWeight;

	}



	//_TIMING(RelaxEngine::timerRelaxInner.start());
	// normalizing all editable weights to totalFreeWeight;
	// make sure sum of all editable weights  = totalFreeWeight, that is, w[n] = w[n]*(totalFreeWeight/sumAllWeights)
	nextWeight = this->nextSkinWeights;
	for (size_t i=0,count=this->geometry.numVertWeights();i<count;i++,nextWeight++){
		if (this->geometry.inflLocked[i])
			// don't touch this weight
			continue;

		if (this->geometry.influencesMask[i]){
			if (isCloseToZero(sumNewWeights))
				*nextWeight = 0;
			else
				*nextWeight *= this->totalFreeWeight/sumNewWeights;
		}
	}

	RelaxEngine &engine = *static_cast<RelaxEngine *>(this->geometry.engine); 
	if (engine.weightLimitEnabled){
		engine.weightsLimiter.applyLimit(this->nextSkinWeights,this->geometry.influencesMask);
		this->normalizeWeights(this->nextSkinWeights);
		this->initFreeWeight(this->nextSkinWeights);
	}
	//_TIMING(RelaxEngine::timerRelaxInner.stop());

}
