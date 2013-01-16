#pragma once

#include <vector>
#include "maya.h"
#include <float.h>
#include "defines.h"
#include <set>
#include "utils.h"

#include "InfluenceTransferInfo.h"
#include "InfluenceMatchRules.h"

#include "types.h"

class VertexTransferInfo {
public:
	unsigned int sourceVertex[3];
	double weights[3];
	
	/**
	 * vertex distance to center is positive for "left" side vertices, and 
	 * negative for "right" side vertices (negative x axis side, for example)
	 */
	double distanceToCenter;

	inline bool isPositiveSide() const {
		return distanceToCenter>0;
	}
	inline bool isNegativeSide() const {
		return distanceToCenter<=0;
	}
};

typedef std::vector<VertexTransferInfo> VertexTransferInfoVec;


class WeightTransferAssociation
{
private:
	InfluenceTransferInfoVec influencesList;
	VertexTransferInfoVec vertexList;
	MPointArray meshPoints;

	SkinToolsGlobal::Axis mirrorAxis;


public:

	WeightTransferAssociation(): mirrorAxis(SkinToolsGlobal::UNDEFINED_AXIS){
	}

	inline const InfluenceTransferInfoVec &getInfluencesList() const {
		return this->influencesList;
	}

	/**
	 * clears mirror data associations; call this before starting addInfluence/initMirror* routines
	 */
	void reset(){
		influencesList.clear();
		vertexList.clear();
	}

	/**
	 * add influence to a set of influences to be associated together as left/right/middle side
	 */
	InfluenceTransferInfo & addInfluence(unsigned int logicalIndex, const MVector &pivot, const MString &name, const MString &path);

	void setVertices(MObject &mesh);

	bool isInitialized() const {
		return (!influencesList.empty() && !vertexList.empty());
	}

	inline const VertexTransferInfo & getVertexTransferInfo(const unsigned int i) const {
		return vertexList.at(i);
	}

	/**
	 * initializes source vertices, taking positions and numbering from given mesh; can be 
	 * different mesh than the one used to initialize vertices of this WTA
	 */
	void initVertexTransferFrom(MObject &mesh,const bool mirror,const SkinToolsGlobal::Axis mirrorAxis=SkinToolsGlobal::UNDEFINED_AXIS);

	inline const SkinToolsGlobal::Axis getMirrorAxis() const{
		return this->mirrorAxis;
	}


	void initInfluenceAssociations(RuleDescriptionList &ruleList, WeightTransferAssociation &sourceWTA);



#ifdef _DEBUG	
	void dumpInfluenceAssociations(){
		DEBUG_COUT_ENDL("Influence Association dump:");
		VECTOR_FOREACH(InfluenceTransferInfo *,influencesList,curr){
			if (!(*curr)->getSource().empty()){
				for (ITIRefVec::const_iterator source=(*curr)->getSource().begin();source!=(*curr)->getSource().end();source++){
					DEBUG_COUT_ENDL("   "<<(*source)->getPath()<<" -> "<<(*curr)->getPath());
				}
			}
			else {
				DEBUG_COUT_ENDL("influence "<<(*curr)->getPath()<<" got no match");
			}
		}
	}
#endif

};
