#pragma once

#include <vector>
#include "maya.h"
#include "timer.h"
#include "utils.h"
#include "WeightedVertex.h"

using namespace std;

class RelaxEngine;
class GeometryInfo;




/**
 * This class is responsible for calculating of one vertex's weight
 */
class VertexRelaxData: public WeightedVertex
{

public:

	VertexRelaxData(GeometryInfo &parent);

	/**
	 * initialize tension to each neighbour.
	 * up to this point, neighbours should hold distance in their
	 * "weight" data, and should have their weights loaded.
	 */
	void initTension();

	/**
	 * run one relax step for this vertex. relax moves weights vector
	 * towards weighted sum of neighbour weights;
	 * each neighbour weight is calculated based on mesh distance
	 *
	 * weight calculation is still experimental.
	 *
	 */
	void performRelaxStep(const double stepSize);

};
