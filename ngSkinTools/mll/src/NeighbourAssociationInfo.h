#pragma once
#include "maya.h"
#include <vector>
#include <map>

/// maps vertexNeigbourID -> link weight
typedef std::map<unsigned int,double> VertexNeighbourWeights;

/**
 * Responsible for calculating and caching vertice neighbourhood weights;
 * for each vertex, a list of vertices that are neighbours of given vertice is stored,
 * and the closer each neighbour is, the higher weight it has
 * 
 * weight sum for each list is 1.0, so it can be easier used in operations where information from all neighbours
 * is blended together
 */
class NeighbourAssociationInfo
{
private:
	std::vector<VertexNeighbourWeights> neighbourWeights;
	MPointArray meshPositions;
	MObject mesh;
public:

	NeighbourAssociationInfo(void);
	~NeighbourAssociationInfo(void);

	/**
	 * clears any cached neighbours and restarts with a given mesh
	 */
	void initialize(MObject &mesh);

	/**
	 * returns neighbour weights per vertex, normalized to sum of 1.0; the closer neighbour is, the higher it's weight
	 */
	VertexNeighbourWeights & getVertexNeighbours(const unsigned int vertex);

};
