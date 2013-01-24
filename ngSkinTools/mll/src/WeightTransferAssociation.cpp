#include "WeightTransferAssociation.h"
#include "defines.h"
#include <maya/MMeshIntersector.h>
#include <maya/MItMeshPolygon.h>
#include <assert.h>

#include "InfluenceMatchRules.h"

/**
 * removes prefix from a string, and returns true, if prefix was indeed found on a string
 */
bool removePrefix(MString &s,const MString &prefix){
	int index = s.indexW(prefix);
	if (index<0)
		return false;

	s = s.substringW(index+prefix.length(),s.length()-1);
	return true;
}

void InfluenceTransferInfo::setDestination(InfluenceTransferInfo &destination){
	if (this->destination!=NULL){
		this->destination->source.erase(this);
	}

	this->destination = &destination;
	this->destination->source.insert(this);
}

InfluenceTransferInfo& WeightTransferAssociation::addInfluence(unsigned int logicalIndex, const MVector &pivot, const MString &name, const MString &path){
	this->influencesList.push_back(new InfluenceTransferInfo());

	InfluenceTransferInfo * const result = influencesList.back();
	result->setPivot(pivot);
	result->setName(name,path);
	result->setLogicalIndex(logicalIndex);

	return *result;
}

void WeightTransferAssociation::initInfluenceAssociations(RuleDescriptionList &ruleList, WeightTransferAssociation &sourceWTA){

	ruleList.resolveAssociations(sourceWTA.influencesList,influencesList);
	
	// unmatched influences are mapped onto themselves in mirroring case
	if (ruleList.isMirrorMode) {

		// each time new destination is set, we are breaking source somewhere else;
		// need to keep searching for empty source lists untill there's none
		// this will eventually stop at the worst case where every influence is mapped onto itself
		bool changesHappened = true;
		while (changesHappened){
			changesHappened = false;
			VECTOR_FOREACH(InfluenceTransferInfo *,influencesList,curr){
				if ((*curr)->getDestination()==NULL){
					(*curr)->setDestination(**curr);
					changesHappened = true;
				}
			}
		}
	}

}

/**
 * returns double the triangle area for triangle v1-v2-v3; 
 */
double triangleArea(const MVector &v1,const MVector &v2,const MVector &v3){
	return ((v2-v1)^(v3-v1)).length();
}

/**
 * calculates barycentric coordinates of a point on a triangle
 */
void calcBaryCoords(const MPointArray &triangle,const MPoint &pointPos,double baryCoords[3]){
	double fullArea = triangleArea(triangle[0],triangle[1],triangle[2]);
	baryCoords[0] = triangleArea(pointPos,triangle[1],triangle[2])/fullArea;
	baryCoords[1] = triangleArea(pointPos,triangle[2],triangle[0])/fullArea;
	baryCoords[2] = 1.0-baryCoords[0]-baryCoords[1];
}

void WeightTransferAssociation::setVertices(MObject &mesh){
	MFnMesh meshFn(mesh);
	vertexList.resize(meshFn.numVertices());
	meshFn.getPoints(meshPoints);
}

void WeightTransferAssociation::initVertexTransferFrom(MObject &sourceMesh,const bool mirror,const SkinToolsGlobal::Axis mirrorAxis) {
	this->mirrorAxis = SkinToolsGlobal::UNDEFINED_AXIS;
	MMeshIntersector intersector;
	intersector.create(sourceMesh);
	
	MItMeshPolygon iterator(sourceMesh);
	MPointOnMesh closestPoint;
	MPointArray trianglePoints;
	MIntArray triangleVertices;
	for (unsigned int i=0;i<meshPoints.length();i++){
		VertexTransferInfoVec::reference mi = vertexList.at(i);

		MPoint pt = meshPoints[i];

		if (mirror) {
			this->mirrorAxis = mirrorAxis;
			switch (mirrorAxis){
				case SkinToolsGlobal::X:
					mi.distanceToCenter = pt.x;
					pt.x = -pt.x;
					break;
				case SkinToolsGlobal::Y:
					mi.distanceToCenter = pt.y;
					pt.y = -pt.y;
					break;
				case SkinToolsGlobal::Z:
					mi.distanceToCenter = pt.z;
					pt.z = -pt.z;
					break;
			}
		}

		intersector.getClosestPoint(pt,closestPoint);
		
		int prevIndex;
		iterator.setIndex(closestPoint.faceIndex(),prevIndex);
		iterator.getTriangle(closestPoint.triangleIndex(),trianglePoints,triangleVertices);
		
		mi.sourceVertex[0] = triangleVertices[0];
		mi.sourceVertex[1] = triangleVertices[1];
		mi.sourceVertex[2] = triangleVertices[2];
		calcBaryCoords(trianglePoints,closestPoint.getPoint(),mi.weights);

	}
}
