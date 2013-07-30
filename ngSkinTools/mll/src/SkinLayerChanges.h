#pragma once
#include "SkinLayerManager.h"
#include "SkinLayerPaintGlobals.h"

// TODO: redo all constructors initialize all required arguments for a change command

namespace SkinLayerChanges {
/**
 * generic parent for all changes made to layers
 * that go together with command into undo/redo queue
 */
class SkinLayerChange  {
public:
	SkinLayerManager * manager;

	virtual void execute()=0;
	virtual void undo()=0;
	virtual void redo()=0;
	virtual void setResult(MPxCommand  &command){};
	virtual ~SkinLayerChange() {};
};

/// "add layer" change
class AddLayer: public SkinLayerChange {
private:
	bool forceEmpty;
public:
	AddLayer(): forceEmpty(false){
	}
	SkinLayer * parent;
	SkinLayer * layer;
	MString newName;
	void execute();
	void undo();
	void redo();
	void setResult(MPxCommand &command){
		command.setResult(static_cast<int>(layer->getID()));
	}

	inline void setForceEmpty(bool forceEmpty){
		this->forceEmpty = forceEmpty;
	}
};

/// "change layer name"
class ChangeLayerName: public SkinLayerChange {
	MString oldName;
public:
	SkinLayer * layer;
	MString newName;
	void execute();
	void undo();
	void redo();
};

/// "change layer opacity"
class ChangeOpacity: public SkinLayerChange {
	double oldOpacity;
public:
	SkinLayer * layer;
	double newOpacity;
	void execute();
	void undo();
	void redo();
};

/// "change layer opacity"
class ChangeEnabled: public SkinLayerChange {
	bool oldEnabled;
public:
	SkinLayer * layer;
	bool newEnabled;
	void execute();
	void undo();
	void redo();
};

class RemoveLayer: public SkinLayerChange {
private:
	SkinLayer * prevParent;
	int layerPosition;
public:
	SkinLayer * layer;
	void execute();
	void undo();
	void redo();
};

class ChangeCurrentLayer: public SkinLayerChange {
private:
	SkinLayer * prevLayer;
	SkinLayer * layer;
public:
	ChangeCurrentLayer(SkinLayer *layer): layer(layer){
	}
	void execute();
	void undo();
	virtual void redo();
};


class ChangeCurrentInfluence: public SkinLayerChange {
private:
	int prevInfluence;
	SkinLayer * layer;
	int influence;
public:
	ChangeCurrentInfluence(const int influence):influence(influence){
	}
	void execute();
	void undo();
	void redo();
};

class ChangeLayerIndex: public SkinLayerChange {
private:
	unsigned int prevIndex;
	SkinLayer &layer;
	unsigned int newIndex;
public:
	ChangeLayerIndex(SkinLayer &layer,const int newIndex):
		layer(layer),
		prevIndex(SkinLayer::LAYER_POSITION_UNDEFINED),
		newIndex(newIndex)
	{

	}
	void execute();
	void undo();
	void redo();
};


class AddDisplay: public SkinLayerChange {
private:
	MDGModifier modifier;
public:
	void execute();
	void undo();
	void redo();
};


class RemoveDisplay: public SkinLayerChange {
private:
	MDGModifier modifier;
public:
	void execute();
	void undo();
	void redo();
};



class AttachData: public SkinLayerChange {
private:
	MDGModifier modifier;
	MDagPath meshPath;
public:
	AttachData(const MDagPath &path): meshPath(path){
	}
	void execute();
	void undo();
	void redo();
};

class MirrorLayer: public SkinLayerChange {
private:
	WeightsChange prevInfluenceWeights;
	WeightsChange prevMaskWeights;
	SkinLayer &layer;
public:
	MirrorLayer(SkinLayer &layer): 
	  layer(layer),
		positiveToNegative(true),
		guessMirrorSide(false),
		mirrorWidth(0.000001),
		mirrorMask(false),
		mirrorWeights(false)
	{
	}
	bool mirrorWeights;
	bool mirrorMask;
	double mirrorWidth;
	bool positiveToNegative;
	bool guessMirrorSide;
	void execute();
	void undo();
	void redo();
};


class FloodWeights: public SkinLayerChange {
private:
	WeightsChange prevWeights;
	SkinLayer & layer;
	double intensity;
	PaintMode paintMode;
public:
	FloodWeights(SkinLayer &layer): layer(layer){
	}
	void execute();
	void undo();
	void redo();
};


class TransparencyToMask: public SkinLayerChange {
private:
	WeightsChange prevMaskWeights;
	WeightsChange prevInfluenceWeights;
	SkinLayer &layer;
public:
	TransparencyToMask(SkinLayer &layer): layer(layer){
	}
	void execute();
	void undo();
	void redo();
};

class MaskToTransparency: public SkinLayerChange {
private:
	WeightsChange prevMaskWeights;
	WeightsChange prevInfluenceWeights;
	SkinLayer &layer;
public:
	MaskToTransparency(SkinLayer &layer): layer(layer){
	}
	void execute();
	void undo();
	void redo();
};

class SetLayerWeights: public SkinLayerChange {
private:
	WeightsChange prevWeights;
	SkinLayer &layer;
	MDoubleArray newWeights;
	int target;
public:
	SetLayerWeights(SkinLayer &layer): layer(layer),target(SkinLayer::PAINT_TARGET_UNDEFINED){
	}

	inline void setTarget(const int target){
		this->target = target;
	}

	inline int getTarget() const {
		return this->target;
	}

	inline void setNewWeights(const MDoubleArray & newWeights){
		this->newWeights = newWeights;
	}

	void execute();
	void undo();
	void redo();
};

class SetMaxInfluencesPerVert: public SkinLayerChange {
private:
	unsigned int previousValue;
	unsigned int value;
public:
	SetMaxInfluencesPerVert(unsigned int value): value(value){
	}


	void execute();
	void undo();
	void redo();
};


}
