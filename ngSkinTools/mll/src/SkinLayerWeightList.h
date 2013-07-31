#pragma once
#include <map>
#include <vector>
#include <assert.h>
#include "WeightTransferAssociation.h"
#include "NeighbourAssociationInfo.h"
#include "exceptions.h"






/**
 * Managed list of weights for all vertices in the mesh;
 * weights are expected to be indexed in the same order as mesh vertices
 * is used in multiple places where "define weights for each vertex" type of 
 * weight is required
 */
class SkinLayerWeightList{
private:
	unsigned int weightCount;

	double  * weights;
public:
	inline const double * const getWeights() const {
		return weights;
	}

	inline void getWeights(MDoubleArray &result) const {
		result.setLength(this->getSize());
		const double * curr = this->getWeights();
		for (unsigned int i=0;i<this->getSize();i++,curr++){
			result.set(*curr,i);
		}
	}

	inline double const getWeight(const unsigned int index) const{
		assert (isInitialized());
		return weights[index];
	}

	inline double * getWeightPtr(const unsigned int index){
		assert(isInitialized());
		return weights+index;
	}

	inline void setWeight(const unsigned int index,const double weight){
		assert (isInitialized());
		assert(weightCount>index);
		weights[index] = weight;
	}

	void setWeights(const MDoubleArray &source);


	/**
	 * attempts to resize weight list; if it's already initialized to equal or bigger size, method does nothing.
	 * otherwise resizes weight list to "size" and sets default value for each item
	 */
	inline void resize(const unsigned int size,const double defaultValue=0.0){

		if (this->weightCount>=size)
			return;

		// allocate new weights array and copy old weights, if there was any
		double *newWeights = new double[size];
		const unsigned int prevSize = this->getSize();
		if (prevSize>0){
			memcpy(newWeights,this->weights,prevSize*sizeof(double));
		}

		// release old memory
		this->release();

		this->weights = newWeights;
		this->weightCount = size;
		
		// for any new allocated weights, set default value
		newWeights += prevSize;
		for (unsigned int i=prevSize;i<size;i++,newWeights++)
			*newWeights=defaultValue;
	}

	/**
	 * uninitalizes memory on this list, returning it into "fresh" state
	 */
	inline void release(){
		if (weights)
			delete [] weights;
		weights = NULL;
		this->weightCount = 0;
	}

	inline const unsigned int getSize() const{
		return this->weightCount;
	}

	SkinLayerWeightList(): weights(NULL),weightCount(0){
	}
	~SkinLayerWeightList(){
		this->release();
	}

	const bool isInitialized() const {
		return weights!=NULL;
	}

	void transferWeights(const SkinLayerWeightList &source,const unsigned int vertexNum,const VertexTransferInfo &mirrorInfo,const double blendWeight=1.0);
	
	void transferWeights(const SkinLayerWeightList &source, const WeightTransferAssociation &destinationWTA);


	void copyFromRange(const SkinLayerWeightList &other,const unsigned int firstVert,const unsigned int lastVert);
	//void copyFromRandomAccess(const SkinLayerWeightList &other,const unsigned int * const randomAccess,const unsigned int numVerts);

	double neighbourSum(const unsigned int vertexNum,NeighbourAssociationInfo &assocInfo){
		double neighbourSum = 0;
		VertexNeighbourWeights &neighbours = assocInfo.getVertexNeighbours(vertexNum);
		for (VertexNeighbourWeights::const_iterator i=neighbours.begin();i!=neighbours.end();i++){
			neighbourSum +=this->getWeight(i->first)*i->second;
		}
		return neighbourSum;
	}


};

class InfluenceWeightsMap {
private:
	// weight map where this instance is adding own weights to
	InfluenceWeightsMap * parentMap;

	/**
	 * always the size of numVerts*numInfluences,
	 * where all weights are listed for first vert, then second vert, etc
	 */
	double * vertWeights;

	unsigned int numVerts;

	/**
	 * indicates reserved memory in .vertWeights. not necessary all memory is used,
	 * that is indicated by logicalToPhysical/physicalToLogical list size.
	 */
	unsigned int numInfluences;

public:
	// logical index for transparency
	static const unsigned int TRANSPARENCY_LOGICAL_INDEX = 0xFFFFFF00;
	// physical transparency index in list
	static const unsigned int TRANSPARENCY_PHYSICAL_INDEX = 0;


	// indexes indicating that no logical or physical influence was defined
	static const unsigned int UNDEFINED_LOGICAL_INFLUENCE = 0xFFFFFF01;
	static const unsigned int UNDEFINED_PHYSICAL_INFLUENCE = 0xFFFFFF00;

	const bool hasTransparencyInfluence;

	InfluenceWeightsMap(const bool hasTransparency);
	~InfluenceWeightsMap();

	

	inline InfluenceWeightsMap * getParentMap(){
		return parentMap;
	}

	inline const unsigned int getNumVerts() const {
		return numVerts;
	}

	inline const unsigned int getNumInfluences() const {
		return numInfluences;
	}

	void normalizeWeights(const int verticeID,const unsigned int skipPhysicalInfluence,const double upscaleTo, const bool discardTransparency);
	

	/// maps "logical influence number in a skin cluster" -> "number of influence in vertWeights"
	std::map<unsigned int,unsigned int> inflLogicalToPhysical;
	/// vice versa of logicalToPhysical field, describes logical index of each influence use by vertWeights memory
	std::vector<unsigned int> inflPhysicalToLogical;

	
	/**
	 * last memorized parent weight array memory pointer; if different value
	 * than parentMap.vertWeights, indicates that parent changed memory
	 * TODO: this mechanic is not reliable
	 */
	double * prevParentWeightsMemory;

	/**
	 * physical-to-physical influence mapping to parent weight list (by first vertex)
	 * e.g., parentMapping[i] points to influence of logical index inflPhysicalToLogical[i]
	 *
	 * to keep indexing compatibility, when having transparency channel,
	 * first pointer is uninitialized
	 */
	double **parentMapping;


	/// marks parent mapping as invalid, to be lazy-initialized on next use
	void invalidateParentMapping();
	
	/// constructs localPhysical->parentPhysical mappings of an influence in parent map
	void initializeParentMapping();
	
	/// weight map, provided as argument, is marked as parent weight map in local data structure
	void setParentMap(InfluenceWeightsMap *map);

	/**
	 * adds influence mapping, but first ensuring that numVerts is set to given value
	 * in case this is the first influence being added
	 * TODO: phase this out; both influence count ant vertex count should be set separately and memory 
	 * initialized once both params are available
	 */
	inline void addInfluenceMapping(const unsigned int numVerts,const unsigned int logical){
		assert(numVerts>0);
		assert(this->numVerts==0 || this->numVerts==numVerts);

		if (this->numVerts==0)
			resize(numVerts, this->numInfluences,false);

		addInfluenceMapping(logical);
	}
	void addInfluenceMapping(const unsigned int logical);

	void resize(const unsigned int numVerts,const unsigned int numInfluences,const bool preserveContents);

	inline bool isInitialized() const{
		return this->vertWeights!=NULL;
	}

	/**
	 * returns array to all weights for all influences; layout is [all weights for vert 1][all weights for vert 2]...
	 * so for influence #2, vertex #3 weight  is at index 3*getNumInfluences()+2
	 *
	 */
	inline double * getVertWeights(){
		return vertWeights;
	}
	
	inline const double * getVertWeights() const {
		return vertWeights;
	}
	inline const double * getVertWeights(const unsigned int vertIndex) const {
		assert(vertIndex<this->numVerts);
		return this->vertWeights+vertIndex*this->numInfluences;
	}

	inline const double * getVertWeights(const unsigned int vertIndex,const unsigned int influence) const {
		assert(vertIndex<this->numVerts);
		return this->vertWeights+vertIndex*this->numInfluences+influence;
	}

	inline double * getVertWeights(const unsigned int vertIndex) {
		return const_cast<double *>(const_cast<const InfluenceWeightsMap*>(this)->getVertWeights(vertIndex));
	}

	inline double * getVertWeights(const unsigned int vertIndex,const unsigned int influence) {
		return const_cast<double *>(const_cast<const InfluenceWeightsMap*>(this)->getVertWeights(vertIndex,influence));
	}

	inline void getInfluenceWeights(const unsigned int influence, MDoubleArray &result) const {
		result.setLength(getNumVerts());
		const double * curr = getVertWeights(0,influence);
		for (unsigned int i=0;i<getNumVerts();i++,curr+=getNumInfluences()){
			result.set(*curr,i);
		}
	}

	inline void getLogicalInfluenceWeights(const unsigned int influence, MDoubleArray &result) const {
		getInfluenceWeights(getPhysicalIndex(influence),result);
	}

	inline const unsigned int getPhysicalIndex(const unsigned int logicalInfluence) const{
		return inflLogicalToPhysical.find(logicalInfluence)->second;
	}

	inline const unsigned int getPhysicalIndexIfExists(const unsigned int logicalInfluence,const unsigned int defaultIndex) const{
		return hasLogicalInfluence(logicalInfluence)?inflLogicalToPhysical.find(logicalInfluence)->second:defaultIndex;
	}

	inline const unsigned int getPhysicalIndexIfExists(const unsigned int logicalInfluence) const{
		return getPhysicalIndexIfExists(logicalInfluence,UNDEFINED_PHYSICAL_INFLUENCE);
	}

	void setInfluenceWeights(const unsigned int influence, MDoubleArray &result);

	inline void setLogicalInfluenceWeights(const unsigned int influence, MDoubleArray &result) {
		addInfluenceMapping(influence);
		setInfluenceWeights(getPhysicalIndex(influence), result);
	}

	/**
	 * sum of vertex weights for every influence;
	 * skips transparency influence, if any
	 */
	inline double vertexWeightSum(const unsigned int vertIndex) const{
		assert(vertIndex<this->getNumVerts());
		double result = 0;
		const double *currWeight = getVertWeights(vertIndex);
		if (this->hasTransparencyInfluence)
			currWeight++;

		for (unsigned int i=(this->hasTransparencyInfluence?1:0);i<numInfluences;i++,currWeight++)
			result += *currWeight;
		return result;
	}

	/**
	 * returns true, if logical influence is registered into this weight list
	 */
	inline bool hasLogicalInfluence(const unsigned int influence) const {
		return (inflLogicalToPhysical.find(influence)!=inflLogicalToPhysical.end());
	}

	inline bool hasPhysicalInfluence(const unsigned int influence) const {
		return inflPhysicalToLogical.size()>influence;
	}

	inline const double * getLogicalInfluence(const unsigned int influence)const{
		assert(hasLogicalInfluence(influence));
		return vertWeights+this->inflLogicalToPhysical.find(influence)->second;
	}

	inline double * getLogicalInfluence(const unsigned int influence) {
		return const_cast<double *>(const_cast<const InfluenceWeightsMap*>(this)->getLogicalInfluence(influence));
	}


	inline const double * getLogicalInfluence(const unsigned int influence,const unsigned int vertex) const {
		return getLogicalInfluence(influence)+vertex*this->numInfluences;
	}

	inline double * getLogicalInfluence(const unsigned int influence,const unsigned int vertex){
		return const_cast<double *>(const_cast<const InfluenceWeightsMap*>(this)->getLogicalInfluence(influence,vertex));
	}


	/**
	 * returns parent influence. note that parent mapping must be first initialized with
	 * initializeParentMapping()
	 *
	 * influence index should be locally physical.
	 */
	inline double * getParentInfluence(const unsigned int influence){
		// never allow requesting of a first parent influence, if we've got a transparency channel (points to uninitialized data)
		assert(!(influence==0 && this->hasTransparencyInfluence));

		return this->parentMapping[influence];
	}
	inline double * getParentInfluence(const unsigned int influence,const unsigned int vertex){
		return getParentInfluence(influence)+vertex*this->parentMap->numInfluences;
	}

	/**
	 * sum of influences weights for each vertice
	 */
	double getInfluenceTotalWeight(const int influence){
		double *w = vertWeights+influence;
		double result = 0.0;
		for (unsigned int i=0;i<numVerts;i++,w+=numInfluences){
			result += *w;
		}
		return result;
	}

	/**
	 * returns true if influence has any weights; indexing is physical
	 */
	inline bool isPhysicalInfluenceUsed(const unsigned int influence) const{
		double *w = vertWeights+influence;
		for (unsigned int i=0;i<numVerts;i++,w+=numInfluences){
			if(*w>0)
				return true;
		}
		return false;
	}

	/**
	 * returns true, if influence, indexed by logical index, is used;
	 * does additional checking if influence exists in mappings, 
	 * prior to looking into actual weights
	 */
	inline bool isLogicalInfluenceUsed(const unsigned int influence) const{
		if (!hasLogicalInfluence(influence))
			return false;
		return isPhysicalInfluenceUsed(inflLogicalToPhysical.find(influence)->second);
	}

	/**
	 * copy a subset of vertices from another weights map,
	 * copying logical<->physical mappings along.
	 *
	 * assumes that structure is empty before copying.
	 */
	void copyFromRange(const InfluenceWeightsMap &other,const unsigned int firstVert,const unsigned int lastVert);

	void copyFromRandomAccess(const InfluenceWeightsMap &other,const unsigned int * const randomAccess,const unsigned int numVerts);

	/**
	 * transfer weights onto the given vertex; weight argument defines how much of new values will overwrite old values.
	 */
	void transferWeights(const InfluenceWeightsMap &fromWeights,const unsigned int vertexNum,const WeightTransferAssociation &mirrorInfo,const double blendWeight=1.0);
	
	void transferWeights(const InfluenceWeightsMap &source, const WeightTransferAssociation &destinationWTA);

	double neighbourSum(const unsigned int vertexNum,const unsigned int physicalInfluence, NeighbourAssociationInfo &assocInfo){
		double neighbourSum = 0;
		VertexNeighbourWeights &neighbours = assocInfo.getVertexNeighbours(vertexNum);
		for (VertexNeighbourWeights::const_iterator i=neighbours.begin();i!=neighbours.end();i++){
			assert(i->first<this->getNumVerts()); 
			neighbourSum +=(*this->getVertWeights(i->first,physicalInfluence))*i->second;
		}
		return neighbourSum;
	}

	/**
	 * every vertex which has it's transparency not at 1.0 is upscaled to 1.0, then every empty vertex (transparency==0) receives
	 * same weighting as it's neighbour;
	 * flooding happens in broad wave algorithm.
	 */
	void floodToTransparency(NeighbourAssociationInfo &assocInfo);

	/**
	 * recalculates transparency influence value
	 */
	inline void recalcTransparency(const unsigned int vertex){
		assert(this->hasTransparencyInfluence);
		*this->getVertWeights(vertex,0) = 1.0-this->vertexWeightSum(vertex);
		
		//require transparency to be above zero, slight error permited
		assert(*this->getVertWeights(vertex,0)>=-SMALL_NUMBER_LAMBDA);
	}

	inline void recalcTransparency(){
		for (unsigned int v=0;v<this->getNumVerts();v++){
			recalcTransparency(v);
		}
	}

	void limitNumberOfInfluences(const int firstVert,const int lastVert, const unsigned int maxInfluences);

};

/**
 * This class defines a set of changes done to layer, and
 * is primarily used to implement undo
 *
 * Typical usage of the class is:
 *	* before changing weights, call any of initialize() methods to "memorize" current weights state
 *  * modify weights in the layer depend	ing on the algorithm
 *  * in the undo() method of outer scope, provide instance of this class
 *    in the layer's restoreWeights() method.
 *
 * Data can be initialized either for a range of vertices, specifying first and last
 * vertice, or by providing a random list of vertice indices
 */
class WeightsChange{
private:
	InfluenceWeightsMap mapValues;
	SkinLayerWeightList listValues;

	InfluenceWeightsMap *destinationMap;
	SkinLayerWeightList *destinationList;
	

	unsigned int firstVert;
	unsigned int lastVert;
	unsigned int destinationSize;

	// contains a list of vertices to memorize; length of the array stored in .size
	const unsigned int *vertRandomAccess;
	unsigned int randomAccessLength;



public:
	WeightsChange();
	~WeightsChange();

	/**
	 * returns vertex range which was updated in this change
	 * can be meaningless if random access vertex array was used instead
	 */
	inline unsigned int getFirstVert() const {return firstVert;}
	inline unsigned int getLastVert() const {return lastVert;}

	/**
	 * returns random vertex access array that was used in this change;
	 * may be null if first-last vert range was used instead
	 */
	inline unsigned const int *getRandomAccess() const {return vertRandomAccess;}
	inline unsigned int getRandomAccessLength() const {return randomAccessLength;} 

	/**
	 * initializes all required data needed for restore()
	 * it is very important that relevant weight change operations
	 * will not add additional influence lists - otherwise, undo will not complete fully
	 */
	void initializeMapRange(const unsigned int firstVert,const unsigned int lastVert,InfluenceWeightsMap &weightMap);
	void initializeMapRandomAccess(const unsigned int * const randomAccess,const unsigned int numVerts,InfluenceWeightsMap &weightMap);
	void initializeListRange(const unsigned int firstVert,const unsigned int lastVert,SkinLayerWeightList &weightList);

	/**
	 * writes back original values
	 * not normally needed to call directly; SkinLayer.restoreWeights() calls this along with other
	 * routines needed to restore weights properly
	 */
	void restore();
};


