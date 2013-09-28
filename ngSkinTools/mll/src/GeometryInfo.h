#pragma once

#include "maya.h"
#include <vector>
#include <map>
#include <set>
#include "defines.h"
#include <maya/MFnMeshData.h>
#include <maya/MMatrix.h>
#include <assert.h>


class WeightedVertex;
class WeightsModifierEngine;

class SkinLayer;
class WeightsChange;

typedef unsigned int LogicalIndex;

/**
 * This class is responsible for holding any per-mesh operations and
 * related data (skin cluster attached to it, or vertex positions),
 *
 * It is also a host for WeightedVertex, managing creation/deletion of it's instances.
 */
class GeometryInfo
{
private:
	/**
     * initializes inflUsage vector
	 */
	void calcUsedInfluences(MDoubleArray &skinWeights,const unsigned int inflCount);

	/**
	 * stores old skin weights at the time of setting new skin weights
	 */
	MDoubleArray oldSkinWeights;

	WeightsChange * layerWeightsChange;
	SkinLayer * layer;

	/**
	 * attemps to initialize geometry info from skinCluster MObject alone
	 */
	void initGeomFn();
	void initSkinWeightsFromLayer();
public:


	/**
     * initializes inflLocked vector
	 */
	void calcLockedInfluences();

	GeometryInfo(WeightsModifierEngine * const engine);
	virtual ~GeometryInfo(void);

	/// points to the engine that owns this geometry processor
	WeightsModifierEngine * const engine;

	/// geometry being edited
	MDagPath path;

	/// geometry world transform
	MMatrix transform;
	
	/// pre-acquired function set for the geometry
	MFnMesh * geomFn;
	
	/// mesh that comes in as geometry data into skin cluster
	MFnMeshData *skinInputGeomData;

	/// skin cluster of this geometry. initialized in init()
	MObject skinCluster;
	/// skin cluster's function set
	MFnSkinCluster * skinFn;

	/**
	 * vertex positions of all vertices in the mesh
	 * data is valid right after init() is called
	 */
	MPointArray vertPositions;

	/**
	 * this vector is always same size as vertex
	 * count in the mesh, but only those elements
	 * that were prior initialized with .initVertex()
	 * are valid
	 */
	std::vector<WeightedVertex *> verts;

	/**
	 * holds list of all influences from the skin cluster for this geometry
	 */
	MDagPathArray influences;

	inline bool getLogicalInfluencePath(const LogicalIndex logicalIndex,MDagPath &path){
		std::map<LogicalIndex,size_t>::iterator i=inflLogicalToPhysical.find(logicalIndex);
		if (i==inflLogicalToPhysical.end())
			return false;

		path = influences[static_cast<unsigned int>(i->second)];
		return true;
	}


	/**
	 * this vector contains as many elements as it is needed to mask
	 * vertex weights vector, marking which elements are used by this particular geometry
	 * weights vector
	 * influences themselves are in the engine->influences list
	 */
	std::vector<bool> influencesMask;

	/**
	 * indicates that weights of this influence should not be altered
	 * uses same indexing as influencesMask (global physical index)
	 */
	std::vector<bool> inflLocked;




	/**
	 * maps influence index from physical to logical.
	 * valid for each influence in local influences list
	 * after init().
	 */
	std::map<size_t,LogicalIndex> inflPhysicalToLogical;

	/**
	 * maps influence index from logical to physical.
	 * valid for each influence in local influences list
	 * after init().
	 */
	std::map<LogicalIndex,size_t> inflLogicalToPhysical;


	/**
	 * contains logical indexes of used influences
	 */
	std::set<LogicalIndex> usedInfluences;

	/**
	 * this maps influence index from locally logical in skin cluster to index in
	 * a list of all influences in the simulation (held by relaxEngine).
	 * in short:
	 *  [logical influence index in skin cluster] -> index in all influences
	 */
	std::map<LogicalIndex,size_t> inflLogicalToGlobal;
	std::map<size_t,LogicalIndex> inflGlobalToLogical;

	/**
	 * returns length of weight arrays in each vertex of this geometry
	 */
	inline size_t numVertWeights() const {
		return this->influencesMask.size();
	}

	inline void addLogicalToGlobalMapping(const LogicalIndex logical,const size_t global){
		assert(usedInfluences.find(logical)==usedInfluences.end());
		usedInfluences.insert(logical);

		while(this->influencesMask.size()<=global)
			this->influencesMask.push_back(false);
		this->influencesMask[global] = true;

		inflLogicalToGlobal.insert(std::pair<LogicalIndex,size_t>(logical,global));
		inflGlobalToLogical.insert(std::pair<size_t,LogicalIndex>(global,logical));
	}



	/**
	 * initializes data in .geomFn, .vertPositions
	 */
	virtual void init();

	/** lazy initialization/access for verts list
	 * initializes (if necessary) element in verts list
	 * and returns it. same as accessing verts[index] for initialized vertexes
	 * 
	 * function returns NULL if vertex number is invalid (index>=vertex count)
	 */
	WeightedVertex * initVertex(const unsigned int index);

	/**
	 * initializes skin weights. should be called after all vertices that
	 * should be present in calculation are initialized
	 */
	void initSkinWeights();

	/**
	 * write calculation results to skin cluster
	 */
	void writeSkinWeights();

	/**
	 * used by undo(): write data to skin cluster that was there prior to any modification
	 */
	void writeOldSkinWeights();

	/**
	 * after weight reading from all custers is done,
	 * this needs to be once again called to finish
	 * filling influence masks and maps
	 */
	void finishInfluenceLists();

	std::set<size_t> invisibleVerts;
	inline const bool isInvisibleVert(const size_t index) const{
		// must be  not initialized and present in invisible verts list
		return (!this->verts[index] && this->invisibleVerts.find(index)!=this->invisibleVerts.end());

	}

	/**
	 * if geometry relies on 
	 */
	inline SkinLayer * getLayer(){
		return this->layer;
	}
};
