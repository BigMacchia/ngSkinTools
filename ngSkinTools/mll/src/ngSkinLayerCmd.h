#pragma once
#include "maya.h"
#include "SkinLayerManager.h"
#include "SkinLayerChanges.h"
#include "StatusException.h"


namespace SkinLayerCmd{
	namespace FlagNames {
		const char ADDLAYER[] = "-add";
		const char FORCEEMPTY[] = "-fe";  // do not initialize from skin cluster, even if applicable
		const char REMOVELAYER[] = "-rm";
		const char LISTLAYERS[] = "-ll";
		const char LISTLAYERINFLUENCES[] = "-lli";
		const char ACTIVEINFLUENCES[] = "-ai";  // only list active influences
		const char CURRLAYER[] = "-cl";
		const char CURRINFLUENCE[] = "-ci";
		const char CURRPAINTTARGET[] = "-cpt";  // named paint target, e.g. "mask"
		const char LAYERNAME[] = "-n";
		const char LAYEROPACITY[] = "-o";
		const char LAYERENABLED[] = "-e";
		const char LAYERPARENT[] = "-p";
		const char LAYERID[] = "-id";
		const char LAYERINDEX[] = "-li"; // queryable, settable: layer index in parent child list
		const char PAINTTARGET[] = "-pt"; // paint target
		const char VERTEXWEIGHTS[] = "-w"; // queryable, settable: vertex weights; supply Layer, paint target and influence ID with the query
		const char INFLUENCEID[] = "-iid";
		const char NUM_CHILDREN[] = "-nch"; // number of children in given layer
		const char DISPLAY_UPDATE[] = "-du";
		const char PAINTINGMODE[] = "-pm"; // notification when maya enters/exits paint mode
		const char COLORDISPLAYNODE[] ="-cdn"; // 1: add display node; 0: remove display node
		const char LAYERDATAATTACH[]="-lda";
		const char LAYERDATAATTACHTARGET[]="-ldt"; // query where, based on current selection, new manager will be attached

		const char TRANSPARENCYTOMASK[]="-ttm"; // convert transparency to mask
		const char MASKTOTRANSPARENCY[]="-mtt"; // convert mask to transparency
		
		const char PAINTOPERATION[]="-po";
		const char PAINTINTENSITY[]="-pi";
		const char PAINTFLOOD[]="-pf";
		

		const char INITMIRRORDATA[]="-imd"; // init mirror data
		const char MIRRORLAYERWEIGHTS[]="-mlw";
		const char MIRRORLAYERMASK[]="-mlm";
		const char MIRRORWIDTH[]="-mw"; // mirror symmetry width
		const char MIRRORAXIS[]="-ma"; // mirroring axis
		const char MIRRORDIRECTION[]="-md"; // mirroring direction
		const char MIRRORCACHEINFO[]="-mcm"; // mirror cache status in readable format
		const char MIRRORCACHEINFLUENCES[]="-mci"; // mirror cache influences associations
		const char INFLUENCEASSOCIATIONDISTANCE[]="-iad"; // distance error when matching left/right influences
		const char INFLUENCEPREFIX[]="-ipf"; // influence prefix list for name match rule
		const char MIRROR_INFLUENCE_ASSOCIATION[]="-maa"; // add/query manual mirror influence association override: source->target
		const char REMOVE_MIRROR_INFLUENCE_ASSOCIATION[]="-rma"; // remove manual mirror influence association override: source->target

		const char COPYSKINDATA[]="-cpd";

		const char VERTCOUNT[] = "-vc"; // query expected vertex count when setting skin weights

		
		// start/end batch updates
		const char BEGINDATAUPDATE[] = "-bdu";
		const char ENDDATAUPDATE[] = "-edu";

		const char INFLUENCE_LIMIT_PER_VERTEX[]="-il"; // global influence limit per vertex; use "0" for no limit

	}


	// valid values for "current paint target" flag
	namespace PaintTargetNames {
		const char INFLUENCE[] = "influence";
		const char MASK[] = "mask";
	}

}

class ngSkinLayerCmd : public MPxCommand
{
private:
	SkinLayerChanges::SkinLayerChange * change;

	SkinLayerManager * layerManager;
	MDagPath selectedShape;

	const MArgDatabase * argData;

	MStatus handleQuery();
	SkinLayerChanges::SkinLayerChange *createUndoableBit();


	void initMirrorData();

	SkinLayer * getOptionalLayer();

	inline void requireManager() const{
		if (layerManager==NULL){
			throwStatusException("no layer info detected",MStatus::kFailure);
		}
	}
	inline void requireInitializedMirrorData() const {
		if (!layerManager->mirrorData.isInitialized())
		{
			throwStatusException("mirror data is not initialized",MS::kInvalidParameter);
		}
	}

	inline SkinLayer * requireLayer() {
		SkinLayer * result = getOptionalLayer();
		if (result==NULL) {
			throwStatusException("target layer is not available",MS::kInvalidParameter);
		}	
		return result;
	}

	inline void requireFlag(const char * const flagName,const char * const errorMessage){
		if (!argData->isFlagSet(flagName)) {
			throwStatusException(errorMessage,MStatus::kInvalidParameter);
		}
	}

	inline SkinLayer * getLayerFromArgId(const char * const flagName,const char * const errorMessage){
		SkinLayerID id;
		argData->getFlagArgument(flagName,0,id);
		SkinLayer *layer = layerManager->getLayerByID(id);
		
		if (layer)
			return layer;

		throwStatusException(errorMessage,MStatus::kInvalidParameter);
		return NULL;
	}

	inline SkinLayer * getLayerFromArgId(const char * const flagName){
		return getLayerFromArgId(flagName,"must supply layer id");
	}

	MSelectionList selectedObjects;
public:
	static const char COMMAND_NAME[];

	static	void* creator();
	static	MSyntax syntaxCreator();
	

	
	MStatus doIt( const MArgList& );
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const;


	ngSkinLayerCmd(void);
	virtual ~ngSkinLayerCmd(void);

	void detectLayerManager();

	inline void setLayerManager(SkinLayerManager &manager){
		this->layerManager = &manager;
	}

	void setSelectedObjects(const MSelectionList &selectedObjects){
		this->selectedObjects = selectedObjects;
	}

	inline const MSelectionList & getSelectedObjects() const {
		return this->selectedObjects;
	}


	void queryManualMirrorInfluenceAssociations(MStringArray &result);

};


