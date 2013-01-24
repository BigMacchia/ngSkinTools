#pragma once

#include <vector>
#include <set>
#include <limits>
#include "maya.h"
#include "defines.h"
#include "utils.h"

#undef max

class InfluenceTransferInfo;

typedef std::set<const InfluenceTransferInfo *> ITIRefVec;

/**
 * Influence Transfer Info:
 * Holds information about one influence: name, position,logical index
 * Each influence can have one destination ("where to" end of weight transfer)
 * and many sources ("where from" end of weight transfer)
 * source/destination information is modified by setting "destination" on the source end
 */
class InfluenceTransferInfo
{
private:
	/// set to true when destination influence is matched by name
	bool nameMatched;

	/// indicates destination distance to currently matched influence
	double matchDistance;

	/// influence pivot point
	MVector pivot;

	/// logical influence index in skin cluster
	unsigned int logicalIndex;

	/// influence name without path
	MString influenceName;

	/// influence full name
	MString influencePath;


	/// list of source influence infos (meaningful when this influences is a destination)
	ITIRefVec source;

	// destination influence (meaningful when this influence is the source)
	InfluenceTransferInfo *destination;
public:

	/// sets destination, and registers self as source into destination's source list
	void setDestination(InfluenceTransferInfo &destination);

	inline InfluenceTransferInfo * getDestination() const{
		return destination;
	}
	

	inline const MVector &getPivot() const {
		return pivot;
	}
	void setPivot(const MVector &pivot){
		this->pivot = pivot;
	}

	inline const unsigned int getLogicalIndex() const {
		return logicalIndex;
	}

	void setLogicalIndex(const unsigned int index){
		this->logicalIndex = index;
	}

	inline const MString &getShortName() const {
		return influenceName;
	}

	inline const MString &getPath() const {
		return influencePath;
	}


	void setName(const MString &name, const MString &fullName){
		influenceName = name;
		influencePath = fullName;
	}

	inline const ITIRefVec & getSource() const {
		return source;
	}
	

	InfluenceTransferInfo():
		destination(NULL),
		matchDistance(std::numeric_limits<double>::max()),
		nameMatched(false)
	{
	}

	InfluenceTransferInfo(const InfluenceTransferInfo &other):
		matchDistance(other.matchDistance),
		nameMatched(other.nameMatched),
		pivot(other.getPivot()),
		logicalIndex(other.getLogicalIndex()),
		influenceName(other.influenceName),
		influencePath(other.influencePath),
		source(other.source),
		destination(other.destination)
	{

	}
};

class InfluenceTransferInfoVec: public std::vector<InfluenceTransferInfo *>{
public:
	const InfluenceTransferInfo * const findByShortName(MString shortName) const {
		VECTOR_FOREACH_CONST(InfluenceTransferInfo *,(*this),i){
			if ((*i)->getShortName()==shortName)
				return *i;
		}
		return NULL;
	}


	virtual ~InfluenceTransferInfoVec(){
		VECTOR_FOREACH(InfluenceTransferInfo *,(*this),i){
			delete *i;
		}
	}
};
