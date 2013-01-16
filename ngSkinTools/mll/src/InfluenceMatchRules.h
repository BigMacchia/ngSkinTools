#pragma once

#include "InfluenceTransferInfo.h"
#include "types.h"
#include <map>


class RuleDescriptionList;

class InfluenceMatchRule
{
private:
	bool matched;

	InfluenceTransferInfo *source;
	InfluenceTransferInfo *match;
protected:
	void setMatch(InfluenceTransferInfo &match){
		this->matched = true;
		this->match = &match;
	}

public:
	RuleDescriptionList * parent;

	InfluenceMatchRule();

	virtual ~InfluenceMatchRule() {
	}

	bool isMirrorMode() const;


	inline bool isMatched(){
		return matched;
	}

	inline InfluenceTransferInfo *getSource(){
		return source;
	}

	inline InfluenceTransferInfo *getMatch(){
		return match;
	}

	virtual void initMatching(InfluenceTransferInfo &source){
		this->source = &source;
		this->matched = false;
		this->match = NULL;
	}

	virtual void testMatch(InfluenceTransferInfo &info)=0;
};

/**
 * Holds a list of rule descriptions, and is responsible for 
 * resolving source-destination associations in InfluenceTransferInfo list
 */
class RuleDescriptionList {
private:
	std::vector<InfluenceMatchRule *> rules;
public:
	void clearRulesList();

	bool isMirrorMode;
	SkinToolsGlobal::Axis mirrorAxis;

	inline void setMirrorAxis(const SkinToolsGlobal::Axis axis){
		this->mirrorAxis = axis;
	}

	inline SkinToolsGlobal::Axis getMirrorAxis() const{
		return this->mirrorAxis;
	}


	bool mirrorPositiveToNegative;

	RuleDescriptionList():
		isMirrorMode(false),
			mirrorAxis(SkinToolsGlobal::X),
			mirrorPositiveToNegative(true)
	{
	}

	void addRule(InfluenceMatchRule * const rule){
		rule->parent = this;
		rules.push_back(rule);
	}

	/**
	 * based on rule list (first rule having the highest priority),
	 * finds best destinations for each of influence in given list
	 */
	void resolveAssociations(std::vector<InfluenceTransferInfo *> &source,std::vector<InfluenceTransferInfo *> &destination);
	~RuleDescriptionList ();
};

class MatchNameRule: public InfluenceMatchRule
{
private:
	MStringArray prefixes;
	/**
	 * removes prefix from a string, and returns true, if prefix was indeed found on a string
	 */
	bool removePrefix(MString &s,const MString &prefix);
	void getStrippedName(const MString &name,MString &strippedName);
public:

	  inline void addPrefix(MString prefix){
		prefixes.append(prefix);
	}
	inline const MStringArray & getPrefixes() const{
		return prefixes;
	}
	virtual void testMatch(InfluenceTransferInfo &info);
};

class MatchDistanceRule: public InfluenceMatchRule
{

private:
	double maxThreshold;

	double bestMatchDistance;
public:
	MatchDistanceRule(): maxThreshold(0)
	{
	}
		

	inline void setThreshold(const double t){
		this->maxThreshold = t;
	}

	virtual void testMatch(InfluenceTransferInfo &info);
	virtual void initMatching(InfluenceTransferInfo &source);
};


class MatchManualOverrideRule: public InfluenceMatchRule
{
private:
	std::map<unsigned int,unsigned int> manualOverrides;
public:
	virtual void addOverride(unsigned int source,unsigned int destination){
		manualOverrides[source]=destination;
	}

	virtual void testMatch(InfluenceTransferInfo &info);
};


