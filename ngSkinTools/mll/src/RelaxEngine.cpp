#include "RelaxEngine.h"
#include "VertexRelaxData.h"
#include "GeometryInfo.h"
#include "StatusException.h"
#include "maya.h"
#include "defines.h"
#include <algorithm>
#include <set>
#include "utils.h"
#include "ClusteredPointCloud.h"
#include "timer.h"
#include "ProgressWindow.h"
#include "GeometryInfo.h"
#include "timer.h"

using namespace std;

#ifdef _SHOW_TIMERS
debug::Timer RelaxEngine::timerRelax;
debug::Timer RelaxEngine::timerRelaxInner;
#endif

RelaxEngine::RelaxEngine(void):
	numSteps(20),
	stepSize(0.1),
	weightLimitEnabled(false)

{
}

RelaxEngine::~RelaxEngine(void)
{

}

WeightedVertex * RelaxEngine::createVertexInfoInstance(GeometryInfo &parent){
	return new VertexRelaxData(parent);
}



void RelaxEngine::initVertTension(){
	// vertList size changes during this loop
	for (uint i=0;i<this->vertList.size();i++){
		static_cast<VertexRelaxData *>(this->vertList[i])->initTension();
	}
}



 



void RelaxEngine::execute(){
	ProgressWindow progress("relax",this->numSteps+6);

	_TIMING(RelaxEngine::timerRelax.reset());
	this->initVertList();
	progress.add();

	this->initInvisibleVerts();
	progress.add();

	// geometries are now ready, we can init vert clouds as neccessary
	this->initVolumeAssociationCloud();
	progress.add();


	this->initVertSoftSelection();
	progress.add();

	this->initVertTension();
	progress.add();

	this->initSkinWeights();
	progress.add();

	
	_TIMING(RelaxEngine::timerRelax.print("Relax Initialization"));

	_TIMING(RelaxEngine::timerRelax.reset());
	_TIMING(RelaxEngine::timerRelaxInner.reset());
	DEBUG_COUT_ENDL("relaxing a total of "<<vertList.size()<<" vertices");
	for (int step=0;step<this->numSteps;step++){

		VECTOR_FOREACH(WeightedVertex *,vertList,v){
			_TIMING(RelaxEngine::timerRelax.start());
			static_cast<VertexRelaxData *>(*v)->performRelaxStep(this->stepSize);
			_TIMING(RelaxEngine::timerRelax.stop());
		}

		VECTOR_FOREACH(WeightedVertex *,vertList,v)
			(*v)->swapWeightsBuffers();
		progress.add();
		if (progress.isCanceled())
			return;
	}

	_TIMING(RelaxEngine::timerRelax.print("Relax"));
	_TIMING(RelaxEngine::timerRelaxInner.print("Relax inner"));
	
	
	this->applySelectionWeights();
	this->finished();
}



