#include <gtest/gtest.h>
#include <glog/logging.h>
#include <iostream>

#include "mayaSupport.h"
#include "../ReferenceMeshHolder.h"

using namespace std;

TEST(ReferenceMeshHolderTest,testSetMesh){
	setupMayaLibrary();
	ReferenceMeshHolder holder;

	MDoubleArray vertices;
	vertices.append(0);vertices.append(0);vertices.append(0);
	vertices.append(1);vertices.append(0);vertices.append(0);
	vertices.append(0);vertices.append(1);vertices.append(0);
	
	MIntArray triangles;
	triangles.append(0);triangles.append(1);triangles.append(2);

	holder.setMesh(vertices,triangles);

}
