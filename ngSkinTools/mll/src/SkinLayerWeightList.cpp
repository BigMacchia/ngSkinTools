#include "SkinLayerWeightList.h"
#include "defines.h"
#include "VertIndexIterators.h"
#include "utils.h"

void SkinLayerWeightList::transferWeights(const SkinLayerWeightList &source,const unsigned int vertexNum,const VertexTransferInfo &mirrorInfo,const double blendWeight) {

	assert(vertexNum<this->weightCount);
	assert(mirrorInfo.sourceVertex[0]<this->weightCount);
	assert(mirrorInfo.sourceVertex[1]<this->weightCount);
	assert(mirrorInfo.sourceVertex[2]<this->weightCount);

	double * currWeight = this->weights+vertexNum;
	*currWeight *= (1-blendWeight);
	*currWeight += blendWeight*(
			source.weights[mirrorInfo.sourceVertex[0]]*mirrorInfo.weights[0]+
			source.weights[mirrorInfo.sourceVertex[1]]*mirrorInfo.weights[1]+
			source.weights[mirrorInfo.sourceVertex[2]]*mirrorInfo.weights[2]);
}

void SkinLayerWeightList::copyFromRange(const SkinLayerWeightList &other,const unsigned int firstVert,const unsigned int lastVert){
	assert(other.getSize()>lastVert);
	this->resize(lastVert-firstVert+1,0.0);
	memcpy(this->weights,other.weights+firstVert,this->getSize()*sizeof(double));
}

void SkinLayerWeightList::transferWeights(const SkinLayerWeightList &source, const WeightTransferAssociation &destinationWTA){
	assert(this->getSize()>0);

	for (unsigned int i=0;i<this->getSize();i++){
		this->transferWeights(source,i,destinationWTA.vertexTransfer.getVertexTransferInfo(i));
	}
}

void SkinLayerWeightList::setWeights(const MDoubleArray &source) {
	// when setting null length, set weights to uninitialized
	if (source.length()==0) {
		this->release();
		return;
	}

	this->resize(source.length());
	for (unsigned int i=0;i<source.length();i++){
		this->setWeight(i,source[i]);
	}
}


InfluenceWeightsMap::InfluenceWeightsMap(const bool hasTransparency):
	hasTransparencyInfluence(hasTransparency),
	numVerts(0),numInfluences(0),vertWeights(NULL),
	parentMapping(NULL),
	parentMap(NULL),
	prevParentWeightsMemory(NULL)
{
	if (hasTransparency)
		this->addInfluenceMapping(InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX);
}


InfluenceWeightsMap::~InfluenceWeightsMap(){
	if (vertWeights)
		delete [] vertWeights;
	if (parentMapping)
		delete [] parentMapping;
}


void InfluenceWeightsMap::addInfluenceMapping(const unsigned int logical){
	// do we have this mapping already?
	if (hasLogicalInfluence(logical))
		return;

	// assume new physical index
	unsigned int physicalIndex = static_cast<unsigned int>(inflLogicalToPhysical.size());
	if (physicalIndex>=this->numInfluences){
		// reallocate memory
		this->resize(this->numVerts,this->numInfluences+1,true);
	}

	// register new mapping
	inflLogicalToPhysical[logical] = physicalIndex;
	inflPhysicalToLogical.push_back(logical);
	invalidateParentMapping();
}

/**
 * resizes the map, preserving weight contents, if required;
 * supports zero-size vertCount, in which case it just registers a new required number of influences
 */
void InfluenceWeightsMap::resize(const unsigned int numVerts, const unsigned int numInfluences,const bool preserveContents){
	// only allow numVerts to be zero if this->numVers is already non-zero
	assert(!(numVerts==0 && this->numVerts!=0));

	// influence count only grows
	assert(!preserveContents || numInfluences>this->numInfluences);
	// vert count should not change when set
	assert(!preserveContents || this->numVerts==0 || this->numVerts==numVerts);

	// allocate new memory, init to zeros.
	double * newWeights = numVerts!=0?(new double[numVerts*numInfluences]):NULL;
	double * currWeight = newWeights;
	for (size_t i=0,count=numVerts*numInfluences;i<count;i++,currWeight++)
		*currWeight = 0.0;

	// initialize transparency to 1.0
	if (this->hasTransparencyInfluence){
		double * currWeight = newWeights;
		for (size_t i=0;i<numVerts;i++,currWeight+=numInfluences){
			*currWeight = 1.0;
		}
	}

	// we already had something allocated? copy old weights and delete memory
	if (this->vertWeights){

		if (preserveContents) {
			// copy weights for each vert
			double * oldWeight = this->vertWeights;
			currWeight = newWeights;
			for (unsigned int v=0;v<numVerts;v++,oldWeight+=this->numInfluences,currWeight+=numInfluences){
				memcpy(currWeight,oldWeight,this->numInfluences*sizeof(double));
			}
		}
		

		delete [] this->vertWeights;
	}

	// save new data and dimensions
	this->vertWeights = newWeights;
	this->numVerts = numVerts;
	this->numInfluences = numInfluences;
}

void InfluenceWeightsMap::copyFromRange(const InfluenceWeightsMap &other, const unsigned int firstVert, const unsigned int lastVert){
	assert(this->hasTransparencyInfluence==other.hasTransparencyInfluence);

	// nothing to copy from
	if (other.getNumVerts()==0)
		return;

	assert(other.getNumVerts()>lastVert);
	inflLogicalToPhysical = other.inflLogicalToPhysical;
	inflPhysicalToLogical = other.inflPhysicalToLogical;
	resize(lastVert-firstVert+1,other.numInfluences,false);
	
	memcpy(vertWeights,other.getVertWeights(firstVert),numInfluences*numVerts*sizeof(double));
}

void InfluenceWeightsMap::copyFromRandomAccess(const InfluenceWeightsMap &other,const unsigned int *const randomAccess,const unsigned int numVerts){
	assert(this->hasTransparencyInfluence==other.hasTransparencyInfluence);

	// nothing to copy from
	if (other.getNumVerts()==0)
		return;

	inflLogicalToPhysical = other.inflLogicalToPhysical;
	inflPhysicalToLogical = other.inflPhysicalToLogical;
	resize(numVerts,other.numInfluences,false);

	const unsigned int * currVert = randomAccess;
	for (unsigned int i=0;i<numVerts;i++,currVert++){
		if (other.getNumVerts()>*currVert)
			// copy memory
			memcpy(this->getVertWeights(i),other.getVertWeights(*currVert),numInfluences*sizeof(double));
		else {
			// initialize zeroes
			for (unsigned int infl=0;infl<numInfluences;infl++){
				*this->getVertWeights(i,infl) = 0.0;
			}
		}
	}
}

void InfluenceWeightsMap::invalidateParentMapping(){
	if (this->parentMapping){
		delete [] parentMapping;
		parentMapping = NULL;
	}
}

void InfluenceWeightsMap::initializeParentMapping(){
	// no parent? can't do anything further
	if (!this->parentMap)
		return;

	// parent changed memory? invalidate mapping
	if (parentMap->vertWeights!=prevParentWeightsMemory)
		invalidateParentMapping();


	// already initialized and valid?
	if (this->parentMapping)
		return;

	// we should not have this function called  in a zero influences situation
	assert(this->numInfluences>0);


	// predict how much memory needs to be allocated
	if (parentMap->numInfluences<inflLogicalToPhysical.size())
		parentMap->resize(parentMap->numVerts,static_cast<unsigned int>(inflLogicalToPhysical.size()),true);
	// force all required influences in parent map
	for (std::vector<unsigned int>::iterator i=inflPhysicalToLogical.begin();i!=inflPhysicalToLogical.end();i++){
		if (*i!=InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX){
			parentMap->addInfluenceMapping(*i);
		}
	}

	prevParentWeightsMemory = parentMap->vertWeights;
	double **curr = this->parentMapping = new double *[this->inflLogicalToPhysical.size()];
	//for (std::map<unsigned int,unsigned int>::iterator i=inflLogicalToPhysical.begin();i!=inflLogicalToPhysical.end();i++,curr++){
	for (std::vector<unsigned int>::iterator i=inflPhysicalToLogical.begin();i!=inflPhysicalToLogical.end();i++,curr++){
		if (*i!=InfluenceWeightsMap::TRANSPARENCY_LOGICAL_INDEX){
			*curr = parentMap->getLogicalInfluence(*i);
		}
	}
}

void InfluenceWeightsMap::setParentMap(InfluenceWeightsMap *map){
	if (this->parentMap==map)
		return;
	this->parentMap = map;
	this->invalidateParentMapping();
}

void InfluenceWeightsMap::transferWeights(const InfluenceWeightsMap &sourceWeights,const unsigned int vertexNum, const WeightTransferAssociation & mirrorInfo,const double blendWeight){
	assert(this->hasTransparencyInfluence==sourceWeights.hasTransparencyInfluence);

	// don't allow copying from same instance
	assert(this!=&sourceWeights);

	const VertexTransferInfo &vmi = mirrorInfo.vertexTransfer.getVertexTransferInfo(vertexNum);

	for (InfluenceTransferInfoVec::const_iterator i=mirrorInfo.getInfluencesList().begin(); i!=mirrorInfo.getInfluencesList().end();i++){
		InfluenceTransferInfo *const info=*i;
		const unsigned int logicalIndex = info->getLogicalIndex();
		
		double newValue = 0;

		
		
		for (ITIRefVec::const_iterator src=info->getSource().begin(),end=info->getSource().end();src!=end;src++){
			const unsigned int sourceInfluence = (*src)->getLogicalIndex();
			if (sourceWeights.hasLogicalInfluence(sourceInfluence)){
				const unsigned int sourcePhysicalIndex = sourceWeights.getPhysicalIndex(sourceInfluence);
				newValue += (*sourceWeights.getVertWeights(vmi.sourceVertex[0],sourcePhysicalIndex))*vmi.weights[0];
				newValue += (*sourceWeights.getVertWeights(vmi.sourceVertex[1],sourcePhysicalIndex))*vmi.weights[1];
				newValue += (*sourceWeights.getVertWeights(vmi.sourceVertex[2],sourcePhysicalIndex))*vmi.weights[2];
			}
		}



		// new value is zero and old one is obviously zero - skip to next iteration
		if (isCloseToZero(newValue) && !this->hasLogicalInfluence(logicalIndex))
			continue;


		this->addInfluenceMapping(logicalIndex);
		double & pValue = *this->getLogicalInfluence(logicalIndex,vertexNum);
		if (blendWeight==1.0) {
			// no need to blend anything, just overwrite
			pValue = newValue;
		}
		else {
			pValue = newValue*blendWeight+pValue*(1.0-blendWeight);
		}

	}
}

void InfluenceWeightsMap::transferWeights(const InfluenceWeightsMap &source, const WeightTransferAssociation &destinationWTA){
	assert(this->getNumVerts()>0);

	for (unsigned int i=0;i<this->getNumVerts();i++){
		this->transferWeights(source, i,destinationWTA);
	}
}

/**
 * this normalization procedure is more complicated than transforming weight list to sum(weights)=1.0.
 *		* only maps that have transparency channel can call normalizeWeights()
 *		* it normalizes all weights in weights[influence0...influenceN][verticeID]
 *		* during normalize, influence specified as skipInfluence will not be touched
 *		* When vertex weight sum is below 1.0, normalization assigns the remaining weight to transparency;
 *		* When vertex weigth sum is above 1.0, normalization sets transaprency to 0.0, and the rest of influences are normalized as usual
 * pass a InfluenceWeightsMap::UNDEFINED_PHYSCICAL_INFLUENCE to skipPhysicalInfluence if no influence should be skipped
*/
void InfluenceWeightsMap::normalizeWeights(const int verticeID,const unsigned int skipPhysicalInfluence,const double upscaleTo, const bool discardTransparency){

	assert(this->hasTransparencyInfluence); // required for what normalize does
	assert(skipPhysicalInfluence!=TRANSPARENCY_PHYSICAL_INDEX); // invalid parameter

	// total weight sum on this vertex
	double weightSum(0.0);

	// skippable weight, if there's a skippable influence defined
	double skipWeightSum(0.0);


	// calculate weightSum and skipWeightSum
	{
		const double *currWeight = getVertWeights(verticeID);
		for (unsigned int i=0;i<this->getNumInfluences();i++,currWeight++){
			if (i==skipPhysicalInfluence){
				// if you're skipping, you better guarantee that it's obeying the rules!
				assert(*currWeight<=1.0);

				skipWeightSum = *currWeight;
			}
			if (i!=TRANSPARENCY_PHYSICAL_INDEX || !discardTransparency){
				weightSum += *currWeight;
			}
		}
	}

	// can't do anything here when all weights are zero here.
	if (weightSum==0)
		return;

	if (isCloseTo(weightSum,skipWeightSum)){
		// condition means that remainder weight = 0.0, so normalize cannot be performed normally;
		// instead, assign the necessary amount to transparency

		// set non-skip weights to zero
		double *currWeight = getVertWeights(verticeID);
		for (unsigned int i=0;i<this->getNumInfluences();i++,currWeight++){
			if (i!=skipPhysicalInfluence)
				*currWeight = 0.0;
		}
		
		// weight is distributed between skipped influence and transparency
		if (discardTransparency){
			*this->getVertWeights(verticeID,TRANSPARENCY_PHYSICAL_INDEX) = 1.0-skipWeightSum;
			return;
		}

		// a somewhat invalid condition here: weight sum is non-zero, but is all skipped; just upscale skipped influence to 1.0,
		*this->getVertWeights(verticeID,skipPhysicalInfluence) = 1.0;
		return;
	}


	// calculate normalization normally
	const double newWeightSum = std::max<double>(upscaleTo,std::min<double>(weightSum,1.0));
	double normalizeCoef = (newWeightSum-skipWeightSum)/(weightSum-skipWeightSum);
	
	double *currWeight = getVertWeights(verticeID);
	for (unsigned int i=0;i<this->getNumInfluences();i++,currWeight++){
		if (i!=skipPhysicalInfluence){
			*currWeight *= normalizeCoef;
		}
	}

	*this->getVertWeights(verticeID,TRANSPARENCY_PHYSICAL_INDEX) = 1.0-newWeightSum;
}

void InfluenceWeightsMap::floodToTransparency(NeighbourAssociationInfo &assocInfo){

	// should only be called on weight maps that have transparency defined as one of influences
	assert(this->hasTransparencyInfluence);

	std::set<unsigned int> borderVertices1,borderVertices2;
	std::set<unsigned int> *currBorder = &borderVertices1;
	std::set<unsigned int> *nextBorder = &borderVertices2;
	std::set<unsigned int> processedVertices;
	
	// part 1: upscale vertices that have *some* transparency in them; skip completely transparent vertices
	for (unsigned int i=0;i<this->getNumVerts();i++){
		const double sumWeights = this->vertexWeightSum(i);
		
		// skip completely transparent vertices
		if (isCloseToZero(sumWeights))
			continue;

		this->normalizeWeights(i,InfluenceWeightsMap::UNDEFINED_PHYSICAL_INFLUENCE,1.0,true);
		processedVertices.insert(i);

		// remove this vertex from border, if it's there
		if (currBorder->find(i)!=currBorder->end()){
			currBorder->erase(i);
		}

		// add neighbours to border, if they're not in processed vertices yet
		VertexNeighbourWeights &neighbours=assocInfo.getVertexNeighbours(i);
		for (VertexNeighbourWeights::iterator vn=neighbours.begin();vn!=neighbours.end();vn++){
			if (processedVertices.find(vn->first)==processedVertices.end()){
				currBorder->insert(vn->first);
			}
		}
		
	}


	// at the moment, processedVertices contain all non-transparent vertices;
	// border vertices contain all vertices that are neighbours to non-transparent vertices
	

	// part 2: start flooding mesh
	while (currBorder->size()>0){
		nextBorder->clear();

		for (std::set<unsigned int>::const_iterator i=currBorder->begin();i!=currBorder->end();i++){
			VertexNeighbourWeights &neighbours=assocInfo.getVertexNeighbours(*i);
			
			// choose neighbour and copy weights from it
			VertexNeighbourWeights::const_iterator bestNeighbour = neighbours.end();
			for (VertexNeighbourWeights::const_iterator vn=neighbours.begin();vn!=neighbours.end();vn++){
				// is neighbour not processed yet? skip it then
				if (processedVertices.find(vn->first)==processedVertices.end())
					continue;

				// if no best neighbour, or this is a better neighbour...
				if (bestNeighbour==neighbours.end() || bestNeighbour->second>vn->second){
					bestNeighbour = vn;
				}
			}

			// we should have found best neighbour here.
			assert (bestNeighbour != neighbours.end());

			// copy weights from best neighbour now
			for (unsigned int infl=0;infl<this->numInfluences;infl++){
				*this->getVertWeights(*i,infl) = *this->getVertWeights(bestNeighbour->first,infl);
			}


			// add it's neighbours to next wave
			for (VertexNeighbourWeights::iterator vn=neighbours.begin();vn!=neighbours.end();vn++){
				if (processedVertices.find(vn->first)==processedVertices.end()){
					nextBorder->insert(vn->first);
				}
			}

		}

		// mark this wave as processed 
		for (std::set<unsigned int>::const_iterator i=currBorder->begin();i!=currBorder->end();i++){
			processedVertices.insert(*i);
		}

		// switch current border now
		currBorder = nextBorder;
		nextBorder = (currBorder==&borderVertices1)?(&borderVertices2):(&borderVertices1);
	}
	
}

void InfluenceWeightsMap::setInfluenceWeights(const unsigned int influence, MDoubleArray &source) {
	// when setting physical influence, size should be defined already
	assert(isInitialized());

	if (!this->hasPhysicalInfluence(influence)){
		throw SkinLayerException("invalid influence specified");
	}

	if (source.length()!=this->getNumVerts()){
		throw SkinLayerException("invalid source vertex count");
	}

	double * curr = getVertWeights(0,influence);
	for (unsigned int i=0;i<getNumVerts();i++,curr+=getNumInfluences()){
		*curr = source[i];
		normalizeWeights(i,influence,0.0,true);
	}


	recalcTransparency();
}


WeightsChange::WeightsChange():
	destinationMap(NULL),
	destinationList(NULL),
	mapValues(true),
	vertRandomAccess(NULL),
	firstVert(0),
	lastVert(0)
{
}

WeightsChange::~WeightsChange(){
	if (this->vertRandomAccess)
		delete [] this->vertRandomAccess;
}

void WeightsChange::initializeMapRange(const unsigned int firstVert, const unsigned int lastVert, InfluenceWeightsMap &weightMap){
	this->firstVert = firstVert;
	this->lastVert = lastVert;
	this->destinationMap = &weightMap;
	this->destinationSize = weightMap.getNumVerts();

	mapValues.copyFromRange(weightMap,firstVert,lastVert);
}

void WeightsChange::initializeMapRandomAccess(const unsigned int * const randomAccess,const unsigned int numVerts,InfluenceWeightsMap &weightMap){
	this->vertRandomAccess = randomAccess;
	this->randomAccessLength = numVerts;
	this->destinationMap = &weightMap;
	this->destinationSize = weightMap.getNumVerts();

	mapValues.copyFromRandomAccess(weightMap,randomAccess,numVerts);
}

void WeightsChange::initializeListRange(const unsigned int firstVert,const unsigned int lastVert,SkinLayerWeightList &weightList){
	assert(lastVert<weightList.getSize() || !weightList.isInitialized());
	this->firstVert = firstVert;
	this->lastVert = lastVert;
	this->destinationList = &weightList;
	this->destinationSize = weightList.getSize();

	if (weightList.isInitialized()){
		this->listValues.copyFromRange(weightList,firstVert,lastVert);
	}
}




void WeightsChange::restore(){
	// get iterator through vertex array
	
	AbstractVertIndexIterator<unsigned int> *iter(NULL);
	
	if (getRandomAccess()){
		iter = new VertIndexIteratorRandom<unsigned int>(getRandomAccess(),this->getRandomAccessLength());
	}
	else {
		iter = new VertIndexIteratorRange<unsigned int>(this->getFirstVert(),this->getLastVert());
	}

	if (destinationMap){
		// restore as map


		// set curr weights to zero
		if (destinationMap->getNumVerts()!=0) {
			for (;!iter->isDone();iter->next()){
				double * currWeight = destinationMap->getVertWeights(iter->index());
				double *end = currWeight+destinationMap->getNumInfluences();
				
				for (;currWeight!=end;currWeight++){
					*currWeight = 0.0;
				}
			}
		}

		// restore old weights now
		for (std::map<unsigned int,unsigned int>::const_iterator it=mapValues.inflLogicalToPhysical.begin();it!=mapValues.inflLogicalToPhysical.end();it++){
			iter->reset();
			for (unsigned int i=0;!iter->isDone();i++,iter->next()){
				*destinationMap->getLogicalInfluence(it->first,iter->index()) = *mapValues.getVertWeights(i,it->second);
			}
		}
	}
	else
	if (this->destinationList){
		// restore as list

		if (listValues.isInitialized()){
			if (!destinationList->isInitialized()){
				destinationList->resize(destinationSize);
			}

			for (unsigned int i=0;!iter->isDone();i++,iter->next()){
				destinationList->setWeight(iter->index(),this->listValues.getWeight(i));
			}
		}
		else {
			destinationList->release();
		}
	}

	delete iter;

}


