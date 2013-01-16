#pragma once
#include <vector>
#include <algorithm>
#include <map>
#include "maya.h"
#include "defines.h"
#include "SkinLayerWeightList.h"
#include "WeightTransferAssociation.h"
#include "SkinLayerPaintGlobals.h"


class SkinLayerManager;


typedef int SkinLayerID;

class SkinLayer
{
public:


	static const int PAINT_TARGET_UNDEFINED = -1;
	static const int PAINT_TARGET_MASK = -2;

	static const int LAYER_POSITION_LAST = -1;
	static const int LAYER_POSITION_UNDEFINED = -2;

private:
	double opacity;
	bool enabled;
	SkinLayerManager & manager;

	/**
	 * layer name. not in any way unique or used internally as identifier.
	 * used purely for user convenience
	 */
	MString name;

	/** 
	 * layer ID is unique for a layer throughout a life off LayerManager, which 
	 * assigns this ID to layer. Think about this like a incremental ID of a database
	 */
	const SkinLayerID id;

	SkinLayer * parent;

	/**
	 * paint target reflects what's being targeted by paint tool. at the moment
	 * positive indexes indicate influences, which are indexed by logical index in this variable
	 * mask and other future indexes reserve negative space, as defined by PAINT_TARGET_* constants
	 */
	int currPaintTarget;
	

public:

	SkinLayer(const SkinLayerID id,SkinLayerManager &manager);
	~SkinLayer(void);

	inline const MString &getName() const{
		return name;
	}
	void setName(const MString &name);

	inline const SkinLayerID getID() const{
		return id;
	}

	inline const double getOpacity() const{
		return opacity;
	}
	inline SkinLayerManager &getManager(){
		return manager;
	}

	inline const SkinLayerManager &getManager() const{
		return manager;
	}

	void setOpacity(const double opacity);

	inline bool isEnabled() const{
		return this->enabled;
	}

	void setEnabled(const bool enabled);

	inline SkinLayer *getParent() const {
		return this->parent;
	}
	void setParent(SkinLayer *parent,int layerPosition=LAYER_POSITION_LAST);

	/**
	 * painted values for individual influences
	 */
	InfluenceWeightsMap influenceWeightList;

	/**
	 * final weight list, calculated for this subtree
	 */
	InfluenceWeightsMap finalWeightList;

	/**
	 * layer mask: mask weight for each vertex
	 */
	SkinLayerWeightList maskWeightList;

	/**
	 * returns currentt paint target
	 */
	inline int getCurrPaintTarget() const {
		return currPaintTarget;
	}

	/**
	 * returns true if current paint target is an influence
	 */
	inline bool currTargetIsInfluence() const {
		return currPaintTarget>=0;
	}

	inline bool currTargetIsDefined() const {
		return currPaintTarget!=SkinLayer::PAINT_TARGET_UNDEFINED;
	}
	
	std::vector<SkinLayer *> children;


	/**
	 * returns an index of a child in this layer
	 */
	inline int getChildIndex(const SkinLayer * const child) const{
		std::vector<SkinLayer *>::const_iterator position = std::find(this->children.begin(),this->children.end(),child);
		if (position==this->children.end())
			return SkinLayer::LAYER_POSITION_UNDEFINED;

		return static_cast<int>(std::distance<std::vector<SkinLayer *>::const_iterator>(children.begin(),position));
	}

	/**
	 * returns self index in a parent list; returns LAYER_POSITION_UNDEFINED, if there's no parent defined, 
	 * or parent does not define this layer as it's child (error condition in this case)
	 */
	inline int getIndex() const{
		if (!parent)
			return SkinLayer::LAYER_POSITION_UNDEFINED;
		return parent->getChildIndex(this);
	}

	/**
	 * returns all current weights; weights are read-only; if there is no current paint target, returns NULL
	 * increment defines a step size needed to loop through all weights in the buffer
	 */
	void getAllCurrWeights(const double** const weights,unsigned int &increment) const;

	inline void setPaintValue(const PaintMode paintMode, const double intensity, const double opacity,const unsigned int vertice,WeightsChange *oldWeights){
		setPaintValue(paintMode, intensity, opacity,vertice,vertice,oldWeights);
	}
	/**
	 * sets painted weight
	 */
	void setPaintValue(const PaintMode paintMode, const double intensity, const double opacity,const unsigned int verticeFrom,const unsigned int verticeTo,WeightsChange *oldWeights);

	/**
	 * restores changed weights; optional second WeightsChange instance is only allowed
	 * when both arguments share same vertex update method (either both same random access, or
	 * same vertex range) - used when restoring a change that affects both mask AND influence weights.
	 *
	 * after restoring changes, method as well updates layer tree and skin cluster with new weights.
	 */
	void restoreWeights(WeightsChange &weightsChange,WeightsChange *weightsChange2=NULL);

	/**
	 * lists influences and their logical skin cluster IDs
	 */
	void listInfluences(MStringArray &inflNames,MIntArray &inflIDs,bool activeOnly);

	/**
	 * sets current paint target
	 */
	void setCurrPaintTarget(const int paintTarget);

	/**
	 * recalculate blended weights of all children
	 */
	void mixChildWeights(const int firstVert,const int lastVert);

	void mixChildWeights();

	void notifyWeightsChanged(const int firstVert,const int lastVert);

	void notifyWeightsChanged();

	inline InfluenceWeightsMap & getFinalWeightList(){
		if (this->children.empty())
			return this->influenceWeightList;

		return this->finalWeightList;
	}

	inline const InfluenceWeightsMap & getFinalWeightList() const {
		return const_cast<SkinLayer*>(this)->getFinalWeightList();
	}

	/**
	 * mixes own weights to given list depending on layer type
	 *	* weighting layers append their own painted weights;
	 *	* masks apply result of their children, masked;
	 */
	void mixOwnWeightsTo(const int verticeID);

	void writeWeightsToSkinCluster(const int firstVert,const int lastVert,const InfluenceWeightsMap &weights);

	void initializeFromSkinCluster();


	void mirrorWeights(const bool toNegativeSide,const double mirrorWidth, const bool mirrorWeights, const bool mirrorMask, WeightsChange * const prevInfluenceWeights,WeightsChange * const prevMaskWeights);
	//void transferWeightsFrom(const InfluenceWeightsMap &source, const WeightTransferAssociation &destinationWTA, WeightsChange * const prevInfluenceWeights,WeightsChange * const prevMaskWeights);

	/**
	 * converts layer transparency to mask
	 */
	void transparencyToMask(WeightsChange * const prevMaskWeights);

	/**
	 * fills transparency with closest weight of weighted vertices;
	 * semi-transparent vertices are updated to full 1.0 weight sum
	 */
	void floodToTransparency(WeightsChange * const prevInfluenceWeights);


	void maskToTransparency(WeightsChange *const prevInfluenceWeights, WeightsChange *const prevMaskWeights);

	/**
	 * a shortcut method to initialize weight changes for influence and mask, if provided
	 */
	void initializeWeightsChanges(WeightsChange *const prevInfluenceWeights, WeightsChange *const prevMaskWeights);

};
