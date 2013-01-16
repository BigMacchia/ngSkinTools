#pragma once
#include "maya.h"
#include <maya/MPxNode.h>

class SkinLayerManager;

class ngSkinLayerDataNode: public MPxNode
{
private:
	SkinLayerManager * manager;
public:
	/// node type id for maya API
	static MTypeId NODEID;

	/// node name in maya
	static char NODENAME[];

	// attributes
	/// layer data read/write link
	static MObject attrData;
	
	/// message attribute linking this data node to skin cluster
	static MObject attrSkinCluster;


	static  void*		creator();
	static  MStatus		initialize();

	void updateLayerData();
public:
	ngSkinLayerDataNode(void);
	virtual ~ngSkinLayerDataNode(void);

	virtual void postConstructor();
	virtual void copyInternalData( MPxNode* pSrc );



	virtual MStatus compute(const MPlug& plug, MDataBlock& dataBlock);

	/**
	 * instantiates and returns manager on demand
	 * can return NULL if manager creation fails for any reason
	 * (skin cluster not connected to node, other data missing etc)
	 */
	SkinLayerManager * getManager();
};
