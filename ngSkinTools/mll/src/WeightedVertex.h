#pragma once

#include <map>
#include "maya.h"

class GeometryInfo;

class WeightedVertex;
/**
 holds information about a link to a neighbour.
 neighbour is a link to another vertex, paired with a weight,
 describing how meaningfull that neighbour is
*/
typedef struct {
	WeightedVertex * neighbour;
	double weight;
} NeighbourAssociation;


typedef std::map<WeightedVertex *,double> WeightedNeigbourMap;
typedef std::pair<WeightedVertex *,double> WeightedNeigbour;



class WeightedVertex
{
private:
	void addNeighbours(WeightedVertex &vert);

	inline const bool containsNeighbour(WeightedVertex * neighbour){
		return neighbours.find(neighbour)!=neighbours.end();
	}

	/**
	 * adds nearby verts connected by surface to association list
	 * argument is usually this same vertex, but due to
	 * welding this might be another vertex
	 */
	void addNeighboursBySurface(WeightedVertex &vert);

	/**
	 * add neighbours by volume to neighbour data.
	 * uses volumeAssociationRadius from relax engine.
	 */
	void addNeighboursByVolume(WeightedVertex &vert);

	/**
	 * perform weilding: inherit more neighbours
	 */
	void weldJump(WeightedVertex &vert);

protected:

	/// a list of associations to neighbour data
	WeightedNeigbourMap neighbours;
	
	/***
	 * nextWeights and originalWeights are two buffers that
	 * should be late-initialized and copied,
	 * just when there will be no more influences participating in the algorithm
	 * best place to call this is right when weights are about to be modified
	 */
	void initializeSecondaryWeightsBuffers();


public:
	WeightedVertex(GeometryInfo &parent);
	virtual ~WeightedVertex(void);

	/**
	 * when vertexes are welded, one becomes parent, others
	 * point to that parent via "welded to"
	 */
	WeightedVertex *weldedTo;

	/**
	 * vertex number in a mesh
	 */
	size_t vertNum;

	/**
	 * vertex weight in soft selection
	 */
	double selectionWeight;

	// data of geometry this vertex belongs to
	GeometryInfo & geometry;

	/**
	 * if set to true, this vert should load weights from skin cluster
	 */
	bool loadsWeights;

	/**
	 * currently valid skin weights
	 */
	double * skinWeights;

	/**
	 * skin weights that are calculated but not yet applied;
	 * make buffer active via swapStepWeights(), so those weights
	 * are chosen when written back to skin cluster
	 */
	double * nextSkinWeights;

	/**
	 * used to restore some of the original weights
	 * for post processing like soft selection
	 */
	double * originalSkinWeights;

	/**
	 * amount of weight on a vertex that is not reserved by locked influences
	 */
	double totalFreeWeight;


	/**
	 * initialize neighbour list according to simulation rules
	 */
	void initNeighbours();

	/**
	 * swaps skinWeights and nextSkinWeights pointer. should be called after whole relax step
	 * is done, so that skinWeights points to updated skin weights
	 */
	void swapWeightsBuffers();

	void applySelectionWeight();

	/**
	 * mixes skin weights and original weights. bias specifies, how much
	 * new weights are kept, e.g., each weight is mixed:
	 * skinWeight = skinWeight*bias+oldWeight*(1-bias)
	 */
	void mixInOriginalWeights(const double bias);

	/**
	 * mixes weights by blend function , which takes new weight and original weight  as parameters
	 */
	void mixInOriginalWeights(double (*blendFunction)(double,double));

	/**
	 * returns position of this point in world coordinates
	 */
	const MPoint vertPosition() const;

	/**
	 * calculates amount of free weight. normally that's 1.0,
	 * but when some influences are locked, the amount of weight
	 * that can be used to redistribute can be smaller
	 */
	void initFreeWeight(double *weights=NULL);

	/**
	 * blindly normalizes weights in skinWeights list
	 */
	void normalizeWeights(double *weights=NULL);


	/**
	 * returns neighbours this vertex should be using
	 */
	inline WeightedNeigbourMap & getNeighbours(){
		if (this->weldedTo){
			return this->weldedTo->neighbours;
		}

		return this->neighbours;
	}

#ifdef _DEBUG
	/**
	 * dumps currently contained weights in this->weights list;
	 * pass another array to displayWeights to see other weights (like .nextWeights)
	 */
	void dumpCurrentWeights(const double threshold=0.01,double *displayWeights=NULL) const;
	std::string name;
	const std::string & getName();
#endif

	/**
	 * when other vertex from another geometry has different influence list,
	 * this one returns multiplier that is needed to upscale common shared weight sum to 1.0
	 * on the other vertex.
	 * e.g., other vertex is weighted [0.5,0.3,0.1,0.1], but only the first and the third influence
	 * is shared between this and the other vertex - this means, multiplier is 1/(0.5+0.1)
	 */
	double getCommonWeightMultiplier(const WeightedVertex & otherVertex);
};
