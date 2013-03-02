#pragma once

#include "maya.h"


class ReferenceMeshHolder {
private:
	MFnMeshData meshData;
	MFnMesh mesh;
public:
	void setMesh(MDoubleArray vertices,MIntArray triangles);

};

