#include "NeighbourAssociationInfo.h"
#include "utils.h"

NeighbourAssociationInfo::NeighbourAssociationInfo()
{
}

NeighbourAssociationInfo::~NeighbourAssociationInfo(void)
{
}

void NeighbourAssociationInfo::initialize(MObject &mesh){
	neighbourWeights.clear();
	meshPositions.clear();
	this->mesh = mesh;
}

VertexNeighbourWeights & NeighbourAssociationInfo::getVertexNeighbours(const unsigned int vertex){
	if (meshPositions.length()==0){
		MFnMesh meshFn(mesh);
		meshFn.getPoints(meshPositions,MSpace::kObject);

		neighbourWeights.resize(meshFn.numVertices());
	}

	VertexNeighbourWeights & result = this->neighbourWeights.at(vertex);

	// already initialized?
	if (result.size()!=0)
		return result;


	MItMeshVertex itMesh(mesh);
	int prevIndex=0;
	MIntArray neighbourIndexes;
	itMesh.setIndex(static_cast<int>(vertex),prevIndex);
	itMesh.getConnectedVertices(neighbourIndexes);

	// add all verts to calculation and make sure they are present in a calculation
	double totalDistance = 0;
	for (unsigned int i=0,count= neighbourIndexes.length();i<count;i++) {
		unsigned int neighbour = neighbourIndexes[i];
		
		// distance is altered slightly to remove zero-distances out of consideration
		double distance = meshPositions[vertex].distanceTo(meshPositions[neighbour])+SMALL_NUMBER_LAMBDA;
		totalDistance += distance;
		result.insert(std::pair<unsigned int,double>(neighbour,distance));
	}

	// inverse weight everything
	double sumWeights = 0;
	for (VertexNeighbourWeights::iterator i=result.begin();i!=result.end();i++){
		i->second = totalDistance/i->second;
		sumWeights += i->second;
	}

	// normalize weights
	for (VertexNeighbourWeights::iterator i=result.begin();i!=result.end();i++){
		i->second /= sumWeights;
	}

	return result;
}
