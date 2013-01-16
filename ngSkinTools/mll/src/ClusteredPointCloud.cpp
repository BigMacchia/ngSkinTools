#include "defines.h"
#include "ClusteredPointCloud.h"

ClusteredPointCloud::ClusteredPointCloud(const double clusterSize)
{
	this->vertexData = NULL;
	this->numVertexData = 0;
	this->it = new ClusteredPointCloudIterator();
	this->it->parent = this;
	this->clusterSize = clusterSize;
}

ClusteredPointCloud::~ClusteredPointCloud()
{
	if (this->vertexData)
		delete [] this->vertexData;
	if (this->it)
		delete this->it;
	for (ClusterMap::iterator it=this->clusters.begin();it!=this->clusters.end();it++)
		delete it->second;
}

void ClusteredPointCloud::addPoint(void * data, const MPoint position){
	VertexCluster::VertexData * curr = &this->vertexData[this->numVertexData];
	this->numVertexData++;

	// initialize vertex data
	curr->data = data;
	curr->position = position;
	curr->next = NULL;

	// get cluster to put this vertex into
	VertexCluster * cluster = this->getCluster(position);

	// push this vert into cluster's list
	if (cluster->verts)
		curr->next=cluster->verts;
	cluster->verts = curr;
}

IPointCloud::IPointIterator * ClusteredPointCloud::getNearbyVerts(const double distance,const MPoint position){
	//DEBUG_COUT_ENDL("querying cloud with "<< this->numVertexData<< " points and "<< this->clusters.size()<<" clusters");
	//DEBUG_COUT_ENDL("at position " << position.x <<" "<<position.y<<" "<<position.z<<", radius: "<<distance);
	this->it->reset(position,distance);
	return this->it;
};

void ClusteredPointCloud::setCapacity(const unsigned int capacity){
	if (this->numVertexData!=0){
		// TODO: capacity setting should throw a special type of exception (like, unsupported operation)
		throw "cannot set another capacity";
	}
	this->vertexData = new VertexCluster::VertexData[capacity];
}

VertexCluster * ClusteredPointCloud::getCluster(const MPoint position){
	// calc index
	ClusterIndex index;
	this->getClusterIndexByCoords(position,index);

	// is there a cluster for this index already?
	ClusterMap::iterator find = this->clusters.find(index);
	if (find!=this->clusters.end())
		return find->second;

	// need to create this cluster
	VertexCluster * newCluster = new VertexCluster();
	this->clusters[index] = newCluster;
	return newCluster;
}

bool ClusteredPointCloud::ClusteredPointCloudIterator::next(){

	// loop while currCluster is not initialized or curr cluster is still non null
	while (!this->currCluster || *this->currCluster){
		// advance forward to next non-null vertex data
		// going further either through vertex list, or clusters,
		// if current vertex list is finished
		if (this->currVertex) {
			this->currVertex = this->currVertex->next;
		}
		if (!this->currVertex){
			// curr vertex is null: we're done with current cluster, continue to next cluster
			// cluster always contains at least one vertex, so there should be no need to iterate further
			if (!this->currCluster)
				this->currCluster = this->clustersToIterate;
			else
				this->currCluster ++;

			// did we just finish with clusters?
			if (!(*this->currCluster))
				return false;

			// grab this vertex queue and start testing it
			this->currVertex = (*this->currCluster)->verts;
		}

		// we got next point! now, need to test if it matches radius criteria;
		// if not, just continue with the loop
		// test position, and yeld this vertex if it matches distance criteria
		this->currDistance =  this->currVertex->position.distanceTo(this->position);
		if (this->currDistance<this->radius){
			return true;
		}

	}


	return false;
}

double ClusteredPointCloud::ClusteredPointCloudIterator::getDistance() const{
	return this->currDistance;
}

void * ClusteredPointCloud::ClusteredPointCloudIterator::getData() const{
	return this->currVertex->data;
}

void ClusteredPointCloud::ClusteredPointCloudIterator::reset(const MPoint position, double radius){
	this->position = position;
	this->radius = radius;

	ClusterIndex hitIndex;
	this->parent->getClusterIndexByCoords(position,hitIndex);

	VertexCluster ** curr = this->clustersToIterate;


	ClusterIndex findIndex;
	ClusterMap::iterator findIterator;
	for (int x=-1;x<=1;x++)
		for (int y=-1;y<=1;y++)
			for (int z=-1;z<=1;z++){
				findIndex.x = hitIndex.x+x;
				findIndex.y = hitIndex.y+y;
				findIndex.z = hitIndex.z+z;
				findIterator = this->parent->clusters.find(findIndex);
				if (findIterator!=this->parent->clusters.end()){
					*curr = findIterator->second;
					curr++;
				}
			}

	//  null-terminate cluster sequence
	*curr = NULL;


	// find first vertex (requirement of vertex iterator)
	this->currCluster = NULL;
	this->currVertex = NULL;


}

VertexCluster::VertexCluster(void)
{
	this->verts = NULL;
}

VertexCluster::~VertexCluster(void)
{
}
