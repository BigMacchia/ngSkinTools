#include <float.h>
#include "InflListerByProximity.h"
#include "WeightedVertex.h"

InflListerByProximity::InflListerByProximity(void)
{
	this->useAllInfluences = true;
}

InflListerByProximity::~InflListerByProximity(void)
{
}

void InflListerByProximity::execute(){
	this->initVertList();

	// TODO: this is very ineficient just to fill in "influences" list
	this->initSkinWeights();

	MPointArray pts;
	VECTOR_FOREACH(WeightedVertex *,this->vertList,i){
		pts.append((*i)->vertPosition());
	}


	// for each influence, calculate it's shortest distance to vertex cloud of each geometry info
	VECTOR_FOREACH(MDagPath,this->influences,infl){
		InfluenceInfo info;
		info.totalWeight = DBL_MAX;

		VECTOR_FOREACH(GeometryInfo *,this->geometries,geom){
			double currDist = static_cast<ProximityGeomInfo *>(*geom)->inflFinder.shortestDistanceToCloud(*infl,pts);
			if (info.totalWeight>currDist){
				info.totalWeight = currDist;
			}
		}

		if (info.totalWeight<DBL_MAX){
			info.influence = *infl;
			this->inflList.push_back(info);
		}
	}

	this->sortByTotalWeight(false);

}
