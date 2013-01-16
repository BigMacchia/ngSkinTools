#pragma once
#include <vector>
#include <map>
#include "IPointCloud.h"
#include "maya.h"



using namespace std;



/**
 * represents one cube in a clustering grid
 */
class VertexCluster
{

public:

	class VertexData{
	public:
		/// data that is caried along with point position
		void * data;
		// position of point in a cloud
		MPoint position;
		// link to next data
		VertexData * next;
	};

	/**
	 * pointer to linked vertex list that are in this cluster.
	 * cluster is not managing memory associated with this data
	 */
	VertexData *verts;

	VertexCluster();
	~VertexCluster();

};


/**
 * represents cluster position in an
 * imaginary xyz grid
 */
class ClusterIndex {
public:
	int x;
	int y;
	int z;

	/**
	 * comparision operator for cluster index
	 */
	class ClusterIndexCompare: public binary_function<ClusterIndex, ClusterIndex, bool>
	{
	public:
		/**
		 * ordering strategy: we first order by X, then Y, then Z
		 */
		bool operator () (const ClusterIndex c1, const ClusterIndex c2) const{
			if (c1.x!=c2.x)
				return c1.x<c2.x;
			if (c1.y!=c2.y)
				return c1.y<c2.y;

			return c1.z<c2.z;
		}
	};
};


/**
 * imaginary cluster grid with a map [X,Y,Z]->VertexCluster
 */
typedef map<ClusterIndex,VertexCluster *,ClusterIndex::ClusterIndexCompare> ClusterMap;


/**
 * Point cloud implementation, using clustering for vertex lookup acceleration
 */
class ClusteredPointCloud: public IPointCloud
{
private:

	/**
	 * iterator implementation for ClusteredPointCloud
	 */
	class ClusteredPointCloudIterator: public IPointCloud::IPointIterator{
		double radius;
		MPoint position;
		static const int GRID_SIZE = 3; // iterate through grid of size N, measured in cluster count
		static const int MAX_CLUSTERS = GRID_SIZE*GRID_SIZE*GRID_SIZE;

		/**
		 * when initialized, this is a null-terminated list,
		 * hence the one more position
		 */
		VertexCluster * clustersToIterate[MAX_CLUSTERS+1];

		/**
		 * current position in clustersToIterate
		 */
		VertexCluster ** currCluster;

		VertexCluster::VertexData *currVertex;
		double currDistance;
	public:
		ClusteredPointCloud * parent;
		void reset(const MPoint position,double radius);
		virtual bool next();
		virtual void * getData() const;
		virtual double getDistance() const;
	};

	/**
	 * list of clusters, lazy initialized via ::getCluster()
	 */
	ClusterMap clusters;


	/**
	 * this strange data structure is a list for all eight possible
	 * combinations of positive/negative X, Y, Z
	 * then each individual cluster is accessed via abs(x)
	 */

	ClusteredPointCloudIterator * it;
	VertexCluster::VertexData * vertexData;
	unsigned int numVertexData;

public:
	double clusterSize;

	ClusteredPointCloud(const double clusterSize);
	~ClusteredPointCloud();
	void addPoint(void * data,const MPoint position);
	IPointCloud::IPointIterator * getNearbyVerts(const double distance,const MPoint position);

	void setCapacity(const unsigned int capacity);

	/**
	 * select cluster by point position
	 */
	VertexCluster * getCluster(const MPoint position);

	/**
	 * converts position in space to cluster index
	 */
	inline void getClusterIndexByCoords(const MPoint coords,ClusterIndex &index){
		index.x = static_cast<int>(floor(coords.x/this->clusterSize));
		index.y = static_cast<int>(floor(coords.y/this->clusterSize));
		index.z = static_cast<int>(floor(coords.z/this->clusterSize));
	}
};
