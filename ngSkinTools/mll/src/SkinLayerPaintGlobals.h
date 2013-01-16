#pragma once
#include <assert.h>
#include "defines.h"

// brush modes 
enum PaintMode {
	pmReplace=1,
	pmAdd=2,
	pmScale=3,
	pmSmooth=4
};


class SkinLayerPaintGlobals {
public:
	/// currently selected paint mode
	static PaintMode currentPaintMode;

	/// current brush intensity
	static double brushIntensity;

	static double brushIntensityReplace;
	static double brushIntensityAdd;
	static double brushIntensityScale;
	static double brushIntensitySmooth;

	static inline void setIntensity(const double intensity){
		SkinLayerPaintGlobals::brushIntensity = intensity;

		switch (SkinLayerPaintGlobals::currentPaintMode){
			case pmReplace:
				SkinLayerPaintGlobals::brushIntensityReplace = intensity;
				break;
			case pmAdd:
				SkinLayerPaintGlobals::brushIntensityAdd = intensity;
				break;
			case pmScale:
				SkinLayerPaintGlobals::brushIntensityScale = intensity;
				break;
			case pmSmooth:
				SkinLayerPaintGlobals::brushIntensitySmooth = intensity;
				break;
			default:
				break;
		}
	}

	/**
	 * operation that alterns given value through opacity with current paint mode and brush intensity
	 * supports replace,add,scale modes; must not be called for other modes
	 */
	static inline void apply(PaintMode mode,double &value,const double opacity,const double intensity){
		assert (mode==pmReplace ||mode== pmAdd ||mode== pmScale);
		switch (mode){
			case pmReplace:
				value = value*(1-opacity)+intensity*opacity;
				break;
			case pmAdd:
				value += intensity*opacity;
				break;
			case pmScale:
				// just a slightly simplified version of: value = value*(1-opacity)+value*brushIntensity*opacity;
				value *= 1-opacity+intensity*opacity;
				break;
			default:
				break;
		}
	}

	/**
	 * as smoothing requires neighbour weight sum, it is implemented as different function
	 */
	static inline void applySmooth(double &value,const double opacity,const double intensity,const double neighbourSum){
		const double smoothIntensity=opacity*intensity;
		value = value*(1-smoothIntensity)+neighbourSum*smoothIntensity;
	}

#ifdef _DEBUG
	/**
	 * returns readable name for current paint mode
	 */
	static const char * const modeName(const PaintMode mode){
		switch (mode){
			case pmReplace:
				return "pmReplace";
			case pmSmooth:
				return "pmSmooth";
			case pmAdd:
				return "pmAdd";
			case pmScale:
				return "pmScale";
		}
		return "<unknown paint mode>";
	}
#endif



};
