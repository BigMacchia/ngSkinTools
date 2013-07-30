#pragma once
#include <vector>
#include <map>
#include <algorithm>
//#include "SkinLayer.h"
#include <maya/MDGModifier.h>
#include "WeightTransferAssociation.h"
#include "SkinLayerWeightList.h"
#include "SkinLayer.h"
#include "NeighbourAssociationInfo.h"
#include "PaintStrokeInfo.h"

class SkinLayer;
class ngSkinLayerDataNode;


class SkinLayerManager;

class DelayedUpdatesStateManager 
{
private:
	bool updateRootWeights;
	int suspendedLevel;
	SkinLayerManager * manager;

	void suspendStarted(){
		updateRootWeights = false;
	}

	void suspendEnded();

public:
	DelayedUpdatesStateManager(): suspendedLevel(0),manager(NULL) {
		
	}

	inline void setManager(SkinLayerManager * manager){
		this->manager = manager;
	}

	inline bool isSuspended() const {
		return suspendedLevel!=0;
	}

	inline void suspend() {
		if (!isSuspended()){
			suspendStarted();
		}
		suspendedLevel++;
		DEBUG_COUT_ENDL("entering manager suspension level "<<suspendedLevel);
	}

	inline void unsuspend() {
		suspendedLevel--;
		DEBUG_COUT_ENDL("exiting to suspension level "<<suspendedLevel);
		assert(suspendedLevel>=0);

		if (!isSuspended()){
			suspendEnded();
		}
	}

	inline void notifyWeightsChanged(SkinLayer &skinLayer) {
		if (this->isSuspended()) {
			this->updateRootWeights = true;
		}
		else {
			skinLayer.notifyWeightsChanged();
		}
	}
	inline void mixChildWeights(SkinLayer &skinLayer) {
		if (this->isSuspended()) {
			this->updateRootWeights = true;
		}
		else {
			skinLayer.mixChildWeights();
		}
	}
};

class SkinLayerManager
{
private:
	int layerIDGenerator;
	std::map<unsigned int,SkinLayer *> layers;
	void listLayers(MStringArray &result,const SkinLayer &parent,const int currDepthLevel) const;
	unsigned int meshVertCount;
	
	// a list of display nodes that are/were autocreated;
	static std::vector<MObjectHandle> autoDisplayNodes;

	unsigned int influenceLimitPerVert;
	
public:
	static const SkinLayerID UNDEFINED_LAYER_ID = -1;

	static void displayColorOnSelection(const bool colorsEnabled);

	static SkinLayerManager * findManager(MDagPath &path);

	/**
	 * finds skin cluster where data node might be attached
	 */
	static MObject findManagerAttachPoint(MDagPath &path);

	/**
	 * finds ngSkinLayerDataNode related to given geometry
	 */
	static ngSkinLayerDataNode * findRelatedDataNode(MObject &attachPoint,MObject *handle);

	
	SkinLayerManager(void);
	virtual ~SkinLayerManager(void);

	SkinLayer * rootLayer;
	/// this flag is turned on when UI is in paint mode
	static bool isPainting;

	DelayedUpdatesStateManager delayedUpdatesState;

	// this flag is set to true, when display needs update,
	// and set to false by color display node when updates mesh
	bool isDisplayDirty;

	bool vertSelectionAvailable;

	PaintStrokeInfo lastStrokeInfo;

	WeightTransferAssociation mirrorData;
	// logical influence indexes of source-destination pairs
	std::map<unsigned int,unsigned int> mirrorManualOverrides;

	bool addMirrorInfluenceAssociation(const MString &source,const MString &destination);
	bool removeMirrorInfluenceAssociation(const MString &source,const MString &destination);

	bool isMirrorCacheValid();

	NeighbourAssociationInfo neighbourInfo;

	/**
	 * contains current weight of selection for each vertex
	 */
	SkinLayerWeightList selectionWeight;

	inline unsigned int getMeshVertCount() const{
		return meshVertCount;
	}

	/**
	 * creates the layer instance and will manage it's memory
	 */
	SkinLayer * createLayer(SkinLayerID id=UNDEFINED_LAYER_ID);

	void listLayers(MStringArray &result) const;


	SkinLayer *getLayerByID(const SkinLayerID id);

	SkinLayer * currentLayer;
	void setCurrentLayer(SkinLayer * const layer);

	MObject meshHandle;
	MObject skinClusterHandle;
	void setSkinCluster(MObject &skinCluster,MObject &mesh);

	void invalidateDisplay();


	bool insertDisplayNode(MDGModifier &modifier);
	static void deleteDisplayNodes(MDGModifier &modifier);

	/**
	 * finds display node, associated with this manager;
	 * returns true, when node was found, and node handle is assigned to passed parameter;
	 * when node is not found, node argument is undefined
	 */
	bool detectDisplayNode(MObject &node);

	void initSelectionWeight();

	void initSkinMirrorData(RuleDescriptionList &ruleList);
	void populateWTA(WeightTransferAssociation &wta);

	void transferWeights(SkinLayerManager &source);

	void findDefaultCurrentLayer();
	
	/**
	 * gets influence path by it's logical index in skin cluster
	 */
	void getInfluencePath(const int logicalInfluenceIndex,MDagPath &path);


	inline void startPaintStroke(){
		lastStrokeInfo.setSize(getMeshVertCount());
		lastStrokeInfo.startStroke(this->mirrorData);
	}

	inline void endPaintStroke(){
		lastStrokeInfo.stopStroke();
	}


	inline const unsigned int getInfluenceLimitPerVert() const{
		return this->influenceLimitPerVert;
	}

	void setInfluenceLimitPerVert(const unsigned int limit);

};

/**
 * scope-bound manager suspend/unsuspend mechanic; define variable to start manager suspension, 
 * and manager will be suspended upon descoping this variable
 */
class LocalManagerSuspension{
private:
	SkinLayerManager &manager;
public:
	LocalManagerSuspension(SkinLayerManager &manager):manager(manager){
		manager.delayedUpdatesState.suspend();
	}
	~LocalManagerSuspension(){
		manager.delayedUpdatesState.unsuspend();
	}
};

#ifdef  _DEBUG
/**
 * this is a test container to provide a manager locator by shape path
 */
class TestManagerLocator {
private:
	MDagPathArray pathList;
	std::vector<SkinLayerManager *> managerList;
public:
	static TestManagerLocator instance;

	void addManager(const MDagPath &path,SkinLayerManager &manager){
		MDagPath shapePath(path);
		Utils::saferExtendToShape(shapePath);
		pathList.append(shapePath);
		managerList.push_back(&manager);
	}

	SkinLayerManager * findManager(const MDagPath &path) const{
		MDagPath shapePath(path);
		Utils::saferExtendToShape(shapePath);

		for (unsigned int i=0;i<pathList.length();i++){
			if (pathList[i]==shapePath)
				return managerList[i];
		}
		return NULL;
	}

	void clear(){
		pathList.clear();
		managerList.clear();
	}

};

#endif
