#define NOMINMAX

#include "InfluenceMatchRules.h"
#include <limits>


InfluenceMatchRule::InfluenceMatchRule():
	matched(false),
	source(NULL),
	match(NULL),
	parent(NULL)
{
	
}


bool InfluenceMatchRule::isMirrorMode() const{
	return parent->isMirrorMode;
}

RuleDescriptionList::~RuleDescriptionList(){
	rules.clear();
}


void RuleDescriptionList::resolveAssociations(std::vector<InfluenceTransferInfo *> &sourceList,std::vector<InfluenceTransferInfo *> &destinationList){
	VECTOR_FOREACH(InfluenceTransferInfo *,sourceList,src){

		VECTOR_FOREACH(InfluenceMatchRule *,rules,dIter){
			InfluenceMatchRule &rule= **dIter;

			rule.initMatching(**src);
			
			// test source against every possible destination and 
			// choose the best one
			VECTOR_FOREACH(InfluenceTransferInfo *,destinationList,dest){
				rule.testMatch(**dest);
			}

			// if destination is found, set it
			if (rule.isMatched()){
				rule.getSource()->setDestination(*rule.getMatch());
				break; // skip remaining rules
			}
		}
	}
}


/**
 * removes prefix from a string, and returns true, if prefix was indeed found on a string
 */
bool MatchNameRule::removePrefix(MString &s,const MString &prefix){
	int index = s.indexW(prefix);
	if (index<0)
		return false;

	s = s.substringW(index+prefix.length(),s.length()-1);
	return true;
}

/**
 * returns a name with any left/right prefix known
 */
void MatchNameRule::getStrippedName(const MString &name,MString &strippedName){
	strippedName = name;
	
	// do not strip names in non-mirror mode.
	if (!isMirrorMode()){
		return;
	}
	
	for (unsigned int i=0;i<prefixes.length();i++){
		if (removePrefix(strippedName,prefixes[i]))
			break;
	}
	
}

void MatchNameRule::testMatch(InfluenceTransferInfo &info){
	// do not match against same influence
	if  (this->getSource()==&info)
		return;

	MString strippedName,otherStrippedName;
	this->getStrippedName(this->getSource()->getShortName(),strippedName);
	this->getStrippedName(info.getShortName(),otherStrippedName);
	if (strippedName==otherStrippedName){
		this->setMatch(info);
	}
}

void MatchDistanceRule::testMatch(InfluenceTransferInfo &info){
	// if source is already a destination for itself, this cannot/should 
	// not be changed by a distance rule
	if (isMatched() && (this->getSource()->getPath()==this->getMatch()->getPath())){
		return;
	}
	
	MVector destinationPivot(info.getPivot());
	if (isMirrorMode()){
		switch(parent->getMirrorAxis()){
			case SkinToolsGlobal::X: destinationPivot.x = -destinationPivot.x; break;
			case SkinToolsGlobal::Y: destinationPivot.y = -destinationPivot.y; break;
			case SkinToolsGlobal::Z: destinationPivot.z = -destinationPivot.z; break;
		}
	}

	// calc distance between points
	destinationPivot -= this->getSource()->getPivot();
	const double distance  = destinationPivot.length();

	// violates max threshold rule?
	if (distance>this->maxThreshold)
		return;


	// we already have a better match?
	if (distance>this->bestMatchDistance)
		return;

	this->bestMatchDistance = distance;
	this->setMatch(info);

}

void MatchDistanceRule::initMatching(InfluenceTransferInfo &source){
	InfluenceMatchRule::initMatching(source);
	this->bestMatchDistance = std::numeric_limits<double>::max();
}


void MatchManualOverrideRule::testMatch(InfluenceTransferInfo &info){
	std::map<unsigned int,unsigned int>::const_iterator i = manualOverrides.find(this->getSource()->getLogicalIndex());
	if (i==manualOverrides.end())
		return;

	if (i->second==info.getLogicalIndex())
		this->setMatch(info);
}



