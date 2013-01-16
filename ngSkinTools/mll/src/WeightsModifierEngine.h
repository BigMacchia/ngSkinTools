#pragma once
#include <vector>
#include <set>
#include "maya.h"


class WeightedVertex;
class IPointCloud;
class GeometryInfo;


typedef std::vector<MDagPath> DagPathVector;


class WeightsModifierEngine
{
private:
	/**
	 * this is an recursive wave function for vertex data structure
	 * it calculates neighbour's new weight, compares it with current neighbour weight,
	 * and proceeds if new weight is smaller for inside verts,
	 * and higher for outside rest
	 * in other words, soft selection is trying to reach as far as it can, both ways
	 */
	void updateSoftSelection(WeightedVertex * vert,const bool inside,const double distanceToWeightRatio);

	bool isExecuteFinished;
protected:


	std::vector<GeometryInfo*> geometries;
	/// a list of vertices that were selected and marked for editing
	std::vector<WeightedVertex*> vertList;

	
	/// same data as vertList, only stored as set.
	std::set<const WeightedVertex*> vertListSet;

	GeometryInfo * findGeometry(MDagPath &path);
	

	/**
	 * initializes vertex soft selection. neighbours must be initialized
	 * with their weights as distances. as a result, more editable vertices
	 * are added during process
	 */
	void initVertSoftSelection();

	/**
	* reads component selection from vertSelection and marks all those verts editable.
	* geometryInfo instances are created and initialized here as well
	*/
	void initVertList();

	/**
	* reads invisible components to geometry infos
	*/
	void initInvisibleVerts();

	/**
	 * invokes applySelectionWeight() for all editable vertices
	 */
	void applySelectionWeights();

	/**
	 * initializes point cloud needed for volume association
	 */
	void initVolumeAssociationCloud();

public:
	/**
	 * set to true if weight information for all influences should be loaded,
	 * instead of only those affecting current selection
	 * this mostly affects allocated space for vertex weight buffers
	 */
	bool useAllInfluences;

	WeightsModifierEngine(void);
	virtual ~WeightsModifierEngine(void);

	/**
	 * descendants can override this method to provide
	 * other instances of WeightedVertex class
	 */
	virtual WeightedVertex * createVertexInfoInstance(GeometryInfo &parent);

	/**
	 * override to instantiate subclasses instead of GeometryInfo 
	 */
	virtual GeometryInfo * createGeometryInfoInstance();


	/**
	 * initializes geometry info node for this geometry path
	 * will return already created instance, if geometry for this path was already initialized,
	 * will return NULL if geometry is not bound to skin cluster
	 */
	GeometryInfo * initGeometry(MDagPath &path);

	/**
	 * all influences being edited across skin clusters.
	 * list is populated by lazy initialization via #getPhysicalIndex
	 */
	DagPathVector influences;

	/**
	 * defines if weights on locked influences should be preserved. will be set by
	 * command by reading user input.
	 */
	bool preserveLockedInfluences;

	/**
	 * passed by command: component selection
	 * to be operated on. this is a raw unvalidated user input
	 */
	MSelectionList vertSelection;

	/**
	 * passed by command: invisible vertices;
	 * that's what should not be "seen" by simulation
	 */
	MSelectionList invisibleVerts;

	/**
	 * volume association radius. zero means we're not using it
	 */
	double volumeAssociationRadius;

	/**
	 * pointcloud itself
	 */ 
	IPointCloud * volumeAssociationCloud;

	/**
	 * populates vert cloud with every vertex from every mesh
	 */
	void initializeVertCloud(IPointCloud *cloud);


	/**
	 * soft selection radius, used to blend resulting weights with new weights
	 * and to modify verts nearby selection 
	 */
	double softSelectionRadius;


	/**
	 *stores calculated skin weights to skin cluster
	 */
	void writeSkinWeights();

	/**
	 * stores skin weights that were originally in skin cluster 
	 * prior to any modification
	 */
	void writeOldSkinWeights();

	/** 
		if needed, stores given influence in a "all influences list", 
		and returns it's index.
		used for lazy initialization of WeightsModifierEngine#influences vector
	*/
	size_t getInfluenceGlobalIndex(const MDagPath &);

	/**
	 * reserves next global index
	 */
	size_t getInfluenceGlobalIndex();

	/**
	 * mark vert editable: appends it to edit procedures
	 */
	void makeEditable(WeightedVertex * vert);
	
	/**
	 * returns true if vertex is marked as editable
	 * (present in global vertList)
	 */
	inline const bool isEditable(const WeightedVertex * const vert)const{
		return (vertListSet.find(vert)!=vertListSet.end());
	}

	/**
	 * initializes skin weights on all geometries:
	 * read weights;
	 * calculate locked influences
	 * normalize current weight values (nothing gets written to skin cluster)
	 * initializes free weight values for vertices
	 */
	void initSkinWeights();

	/**
	 * engines should follow a configure->execute->retreive results pattern.
	 * this is a part of execute entry point
	 * at the end of successfull execute, "execute finished" should 
	 */
	virtual void execute()=0;

	/**
	 * run this at the end of a successfull execute
	 */
	void finished();

	/**
	 * returns true if this engine has an undoable operation
	 */
	virtual bool canUndo() const;

	/**
	 * when set to true, will use maya's soft selection mechanism instead of own
	 */
	bool useExternalSoftSelection;

	inline const std::vector<WeightedVertex*> & getVertList() const {
		return this->vertList;
	}

};
