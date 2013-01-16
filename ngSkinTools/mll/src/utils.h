#pragma once
#include "maya.h"


const double SMALL_NUMBER_LAMBDA = 0.000001;

/**
 * checks if two numbers are close to each other (max allowed difference is SMALL_NUMBER_LAMBDA)
 * if second number is passed as a compile-time constant, speedup is expected
 */
inline bool isCloseTo(const double number,const double number2) {
	return number<(number2+SMALL_NUMBER_LAMBDA) && number>(number2-SMALL_NUMBER_LAMBDA);
}


/**
 * checks if provided number is nearly zero; just a separate case of isCloseTo() function
 */
inline bool isCloseToZero(const double number){
	return isCloseTo(number,0.0);
}


/**
 * return clamped value between min and max
 */
template <typename T>
inline T clamp(const T value,const T min, const T max){
	return value<min?min:(max<value?max:value);
}


/**
 * convert comma separated double list into double array; e.g.
 * "1.2,1.5,1.6" -> [1.2, 1.5, 1.6]
 */
void mStringToDoubleArray(const MString &src, MDoubleArray &result);


class Utils
{
public:
	/**
	 * finds nearest skin cluster attached to given mesh.
	 * returns empty MObject if no skin cluster is found
	 */
	static MObject findSkinCluster(MDagPath mesh);

	/**
	 * extends path to shape, but ensuring that
	 * shape is not an intermediate object
	 * does nothing for shape paths
	 * returns false if extending to shape could not be done
	 */
	static bool saferExtendToShape(MDagPath &path);

	/**
	 * takes string, which contains node names, separated by '/', and
	 * creates a selection list from that
	 */
	static void getSelectionFromString(const MString sel, MSelectionList &selection);
	
	/**
	 * finds child attribute in given parent attribute
	 */
	static bool findChildAttr(MObject &parentAttr,MString childName,MObject &childAttr);

	/**
	 * finds a shape object where source plug is connected
	 */
	static bool findOutputMesh(MPlug &sourcePlug,MObject &shape);

	static void enableDisplayColors(MObject &meshNode,const bool enable);

	/**
	 * returns true if shift key is pressed
	 */
	static bool shiftPressed();

	/**
	 * returns true if ctrl key is pressed
	 */
	static bool ctrlPressed();

	/**
	 * find influence by name in skin cluster
	 */
	static inline MStatus findInfluenceInSkinCluster(MObject &skinCluster,const MString &path,unsigned int & logicalIndex){
		MFnSkinCluster clusterFn(skinCluster);
		MSelectionList selection;
		CHECK_MSTATUS_AND_RETURN_IT(selection.add(path));

		MDagPath dagPath;
		CHECK_MSTATUS_AND_RETURN_IT(selection.getDagPath(0,dagPath));

		MStatus status;
		logicalIndex = clusterFn.indexForInfluenceObject(dagPath,&status); CHECK_MSTATUS_AND_RETURN_IT(status);
		return MStatus::kSuccess;
	}

};
