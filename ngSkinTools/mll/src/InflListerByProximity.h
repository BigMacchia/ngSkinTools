#pragma once
#include "InflListerBase.h"
#include "GeometryInfo.h"
#include "ClosestInfluenceFinder.h"

class InflListerByProximity;

class ProximityGeomInfo: public GeometryInfo
{
public:
	ClosestInfluenceFinder inflFinder;

	ProximityGeomInfo(InflListerByProximity * engine): GeometryInfo(reinterpret_cast<WeightsModifierEngine *>(engine)) {
	}

	virtual void init(){
		GeometryInfo::init();
		inflFinder.setMeshInfo(&this->skinCluster, this->geomFn,this->skinFn);
		for (unsigned int i=0,count=this->influences.length();i<count;i++){
			inflFinder.precalcInfluenceShape(this->influences[i],&this->transform);
		}
		
	}
};

class InflListerByProximity :
	public InflListerBase
{
public:
	InflListerByProximity(void);
	virtual ~InflListerByProximity(void);

	virtual GeometryInfo * createGeometryInfoInstance(){
		return new ProximityGeomInfo(this);
	}

	virtual void execute();
};
