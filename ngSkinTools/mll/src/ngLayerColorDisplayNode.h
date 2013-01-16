#pragma once
#include "maya.h"
#include <maya/MPxNode.h>


class SkinLayer;

/**
 * weight color display node, inserted similarly as a deformer,
 * taking an inMesh and coloring it with it's associated skin layer data;
 * returns outMesh, identical to an inMesh, except with vertex 
 * color data modified
 */
class ngLayerColorDisplayNode: MPxNode
{
private:
	void displayLayerWeights(const SkinLayer &layer);
	SkinLayer * getConnectedSkinLayer();
	MColorArray currColors;
	MIntArray vertexIndexes;

	void resizeVertexIndexes(const unsigned int newSize);
public:
	void displayCurrentWeights(const SkinLayer &layer);

	static MObject attrInMesh;
	static MObject attrOutMesh;
	static MObject attrInEvalTrigger;

	static const MString NODENAME;
	static const MTypeId NODEID;
	ngLayerColorDisplayNode(void);
	virtual ~ngLayerColorDisplayNode(void);
	static  void*		creator();
	static  MStatus		initialize();

	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock);

	virtual MPlug passThroughToOne(const MPlug&plug) const;
	inline const MColorArray & getCurrColors() const {
		return currColors;
	}
};
