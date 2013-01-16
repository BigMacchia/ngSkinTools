#include "MakeRigidWeights.h"
#include "ProgressWindow.h"
#include "GeometryInfo.h"

void RigidWeightsVertex::solveCluster(){
	if (this->clusterRoot!=NULL)
		return;

	this->clusterRoot = this;

	// cluster data
	RigidWeightsVertex *currentTail = this;

	if (static_cast<MakeRigidWeights * const>(this->geometry.engine)->isSingleClusterMode){
		this->createSingleCluster();
	}
	else {
		this->continueClusterNeighbours(&currentTail);
	}

	// calculate new weights
	this->initializeSecondaryWeightsBuffers();

	for (RigidWeightsVertex * currCluster = this->clusterNext;currCluster!=NULL;currCluster=currCluster->clusterNext){
		double * nextWeights = this->skinWeights;
		double * currWeights = currCluster->skinWeights;
		for (size_t i=0,count=this->geometry.numVertWeights();i<count;i++,nextWeights++,currWeights++){
			if (this->geometry.influencesMask[i])
				*nextWeights += *currWeights;
		}
	}

	this->normalizeWeights();

	for (RigidWeightsVertex * currCluster = this->clusterNext;currCluster!=NULL;currCluster=currCluster->clusterNext){
		currCluster->initializeSecondaryWeightsBuffers();
		memcpy(currCluster->skinWeights,this->skinWeights,this->geometry.numVertWeights()*sizeof(double));
	}
}

void RigidWeightsVertex::createSingleCluster(){
	this->clusterRoot = this;

	RigidWeightsVertex *prev = this;
	VECTOR_FOREACH_CONST(WeightedVertex *,this->geometry.engine->getVertList(),i){
		RigidWeightsVertex * curr = static_cast<RigidWeightsVertex *>(*i);
		if (curr==this)
			continue;

		// skip non-editable vertices
		if (!this->geometry.engine->isEditable(curr)){
			continue;
		}

		if (prev==NULL){
			curr->clusterRoot = curr;
		}
		else {
			prev->clusterNext = curr;
			curr->clusterRoot = prev->clusterRoot;
		}
		prev = curr;
	}
}

void RigidWeightsVertex::continueClusterNeighbours(RigidWeightsVertex **currentTail){
	MStatus status;
	MObject meshObj(this->geometry.skinInputGeomData->object());
	MItMeshVertex itMesh(meshObj,&status);
	int prevIndex=0;
	MIntArray neighbourIndexes;
	itMesh.setIndex(static_cast<int>(this->vertNum),prevIndex);
	itMesh.getConnectedVertices(neighbourIndexes);

	for (unsigned int i=0,count=neighbourIndexes.length();i<count;i++){
		RigidWeightsVertex * w = static_cast<RigidWeightsVertex *>(this->geometry.initVertex(neighbourIndexes[i]));
		if (!w->clusterRoot && this->geometry.engine->isEditable(w)){
			// extend cluster
			w->clusterRoot = this->clusterRoot;
			(*currentTail)->clusterNext = w;
			*currentTail = w;
			w->continueClusterNeighbours(currentTail);
		}

	}


}


MakeRigidWeights::MakeRigidWeights(void):
	isSingleClusterMode(false)
{
}

MakeRigidWeights::~MakeRigidWeights(void)
{
}



void MakeRigidWeights::execute(){
	this->initVertList();
	this->initVertSoftSelection();
	this->initSkinWeights();


	// 1. calculate vertex clustering
	//    this could be done by simply chaining vertex infos
	// 2. calculate cluster weights
	// 3. apply weights in the cluster
	ProgressWindow progress("Rigid Weights",static_cast<const int>(this->vertList.size()));

	VECTOR_FOREACH(WeightedVertex *,this->vertList,i){
		static_cast<RigidWeightsVertex *>(*i)->solveCluster();
		progress.add();
		if (progress.isCanceled())
			return;
	}


	this->applyEffectIntensity();
	this->applySelectionWeights();
	this->finished();
}
