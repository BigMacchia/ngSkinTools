#include "ReferenceMeshHolder.h"
#include "StatusException.h"

void ReferenceMeshHolder::setMesh(MDoubleArray vertices, MIntArray triangles){
	if (vertices.length()%3!=0)
		throwStatusException("Invalid number of coordinates in vertices list",MStatus::kInvalidParameter);
	
	if (triangles.length()%3!=0)
		throwStatusException("Invalid number of IDs in triangles list",MStatus::kInvalidParameter);


	MStatus status;
	MObject meshParent = meshData.create(&status);
	CHECK_STATUS("could not create mesh data parent",status);

	MPointArray points;
	for (unsigned int i=0;i<vertices.length();i+=3){
		points.append(vertices[i],vertices[i+1],vertices[i+2]);
	}

	MIntArray triangleCounts;
	for (unsigned int i=0;i<triangles.length()/3;i++){
		triangleCounts.append(3);
	}
	

	mesh.create(static_cast<int>(points.length()),static_cast<int>(triangleCounts.length()),points,triangleCounts,triangles,meshParent,&status);
	CHECK_STATUS("could not create mesh data",status);
}