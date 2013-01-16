/*


A set of maya commands specifically used as callbacks to artUserPaint.


*/

#pragma once


#include <vector>
#include "maya.h"
#include "SkinLayerWeightList.h"
#include "SkinLayer.h"

class SkinLayerManager;

namespace LayerPaintCmds {

	class LayerPaintStrokeInfo{
	private:
		std::vector<SkinLayerManager*> managers;
	public:
		static const int NO_MANAGER = -1;
		void clear();
		int addSurface(MString surface);
		SkinLayerManager * getManager(const int ID) const;
	};


	class GetValueCmd : public MPxCommand
	{
	public:
		static const char COMMAND_NAME[];

		static		void* creator(){
			return new GetValueCmd();
		}
		static		MSyntax syntaxCreator();	

		MStatus doIt( const MArgList& args );
		bool isUndoable() const {return false;}
	};

	class SetValueCmd : public MPxCommand
	{
	private:
		WeightsChange prevWeights;
		SkinLayer * layer;
		int vertexID;
		double opacity;
		double intensity;
		PaintMode mode;

		static bool keyboardOverrideSmooth;
		static bool keyboardOverrideInverse;
	public:
		
		/**
		 * stores brush modifiers from keyboard for later use
		 * should be called at the start of each stroke
		 */
		static void cacheKeyboardState(){
			DEBUG_COUT_ENDL("caching keyboard state");
			keyboardOverrideSmooth = Utils::shiftPressed();
			keyboardOverrideInverse = Utils::ctrlPressed();
		}

		static const char COMMAND_NAME[];

		static		void* creator(){
			return new SetValueCmd();
		}
		static		MSyntax syntaxCreator();	

		MStatus doIt( const MArgList& args );
		bool isUndoable() const {return true;}
		MStatus redoIt(){
			layer->setPaintValue(this->mode,this->intensity,this->opacity,this->vertexID,&this->prevWeights);
			return MS::kSuccess;
		}
		MStatus undoIt(){
			layer->restoreWeights(this->prevWeights);
			return MS::kSuccess;
		}

	};

	class InitializeCmd : public MPxCommand
	{
	public:
		static const char COMMAND_NAME[];

		static		void* creator(){
			return new InitializeCmd();
		}
		static		MSyntax syntaxCreator();	

		MStatus doIt(const MArgList& args);
		int doIt(const MString &shapeName);

		// fake undo just so we sit in the undo stack happily
		bool isUndoable() const {return true;}
		MStatus redoIt(){return MS::kSuccess;}
		MStatus undoIt(){return MS::kSuccess;}
		

	};

	class FinalizeCmd : public MPxCommand
	{
	public:
		static const char COMMAND_NAME[];

		static		void* creator(){
			return new FinalizeCmd();
		}
		static		MSyntax syntaxCreator();	

		MStatus doIt( const MArgList& args );
		void doIt(const int id);
		bool isUndoable() const {return false;}
	};
}

