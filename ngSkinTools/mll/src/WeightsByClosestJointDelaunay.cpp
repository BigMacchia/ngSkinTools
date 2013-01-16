#include <maya/MFnNurbsCurve.h>

#include "WeightsByClosestJointDelaunay.h"
#include "tetgen/tetgen.h"
#include "ClosestInfluenceFinder.h"
#include "utils.h"
#include "GeometryInfo.h"


WeightsByClosestJointDelaunay::WeightsByClosestJointDelaunay(void)
{
}

WeightsByClosestJointDelaunay::~WeightsByClosestJointDelaunay(void)
{
}

bool WeightsByClosestJointDelaunay::isPointAdded(const MPoint &pt){
	for (unsigned int i=0;i<this->addedPoints.length();i++){
		if (this->addedPoints[i].isEquivalent(pt,SMALL_NUMBER_LAMBDA))
			return true;
	}
	return false;
}

inline double randomJitter(){
	return static_cast<double>(rand()/RAND_MAX)*SMALL_NUMBER_LAMBDA*2;
}

void WeightsByClosestJointDelaunay::addPoint(MPoint &pt){
	MPoint newPoint = pt;
	while (this->isPointAdded(newPoint)){
		newPoint.x = pt.x+randomJitter();
		newPoint.y = pt.y+randomJitter();
		newPoint.z = pt.z+randomJitter();
	}
	this->addedPoints.append(newPoint);
}


void WeightsByClosestJointDelaunay::execute(){
	this->initVertList();
	this->initVertSoftSelection();
	this->initSkinWeights();

	tetgenio delaunayInput,additionalPoints, delaunayOutput,delaunayOutput2;

	additionalPoints.numberofpoints =2;
	additionalPoints.pointlist = new REAL[additionalPoints.numberofpoints * 3];
	
	REAL *pt = additionalPoints.pointlist;
	pt[0] = 0;
	pt[1] = 6.455;
	pt[2] = 2.641;
	
	pt += 3;
	pt[0] = 0;
	pt[1] = 3.403;
	pt[2] = -1.805;
	



//#define useFaces

	// count number of elements
	VECTOR_FOREACH(GeometryInfo *,this->geometries,i){
		GeometryInfo *g = *i;
		delaunayInput.numberofpoints += g->vertPositions.length();
		
#ifdef useFaces
		delaunayInput.numberoffacets += g->geomFn->numPolygons();
#endif	
	} // geom foreach


	DEBUG_COUT_ENDL("DELAUNAY INCOMMING!");

	// memory allocation
	delaunayInput.pointlist = new REAL[delaunayInput.numberofpoints * 3];
	delaunayInput.pointattributelist = new REAL[delaunayInput.numberofpoints];
	delaunayInput.numberofpointattributes = 1;
#ifdef useFaces
	delaunayInput.facetlist = new tetgenio::facet[delaunayInput.numberoffacets];
	delaunayInput.facetmarkerlist = new int[delaunayInput.numberoffacets];
#endif

	// fill in elements
	REAL * currPoint = delaunayInput.pointlist;
	REAL * currPointAttr = delaunayInput.pointattributelist;
	int triangleIndexingOffset = 0;
	tetgenio::facet *f = delaunayInput.facetlist;
	VECTOR_FOREACH(GeometryInfo *,this->geometries,i){
		GeometryInfo *g = *i;
		for (unsigned int pt=0;pt<g->vertPositions.length();pt++){
			MPoint position = g->vertPositions[pt]*g->transform;
			currPoint[0] = position.x;
			currPoint[1] = position.y;
			currPoint[2] = position.z;
			currPoint +=3;
			currPointAttr +=1;
		}

#ifdef useFaces
		for (unsigned int poly=0,polycount=g->geomFn->numPolygons();poly<polycount;poly++,f++){
			// make facet

			// get polygon vertice ids
			MIntArray verts;
			g->geomFn->getPolygonVertices(poly,verts);

			// add one polygon
			f->numberofpolygons = 1;
			f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
			f->numberofholes = 0;
			f->holelist = NULL;
			tetgenio::polygon * p = &f->polygonlist[0];

			// fill polygon info
			p->numberofvertices = verts.length();
			p->vertexlist = new int[p->numberofvertices];
			for (unsigned int pv=0;pv<verts.length();pv++){
				p->vertexlist[pv] = triangleIndexingOffset+verts[pv];
			}

		}
#endif

		triangleIndexingOffset += g->vertPositions.length();
	}


	

	DEBUG_COUT_ENDL("starting tetrahedralize");
	//command.edgesout = 2;
	//delaunayInput.save_nodes("F:\\yousuckNodes.whatever");
	//delaunayInput.save_poly("F:\\yousuckPoly.whatever");
#ifdef useFaces
	tetrahedralize("pq1.414a2.0i", &delaunayInput,&delaunayOutput,&additionalPoints);
#else
	tetgenbehavior command;
	command.btree = false;
	command.quiet = false;
	command.verbose = true;
	command.diagnose = false;
	command.facesout = false;
	command.plc = false;
	command.refine = false;
	command.insertaddpoints = true;

	tetrahedralize(&command, &delaunayInput,&delaunayOutput,&additionalPoints);

	// this two-pass call creates a refined mesh that has
//	tetrahedralize(&command, &delaunayInput,&delaunayOutput2,&additionalPoints);
//	tetrahedralize("ra1.0", &delaunayOutput2,&delaunayOutput);
#endif


	
	DEBUG_COUT_ENDL("tetra output");
	MFnNurbsCurve curve;
    MPointArray controlVertices;
    MDoubleArray knotSequences;
	MFnDependencyNode curveParent;

	curveParent.create("transform");
	curveParent.setName("curveLol");


#ifdef _DEBUG
// how to output debug meshes?
//#define outputTrifaces
#define outputTetrahedra
#endif

#ifdef outputTetrahedra
	int *currTetra = delaunayOutput.tetrahedronlist;
	for (int i=0;i<delaunayOutput.numberoftetrahedra;i++,currTetra+=4){
		for (int j=0;j<4;j++){
			//WeightedVertex * pt1 = reinterpret_cast<WeightedVertex *>(static_cast<unsigned>(delaunayOutput.pointattributelist[currTetra[j]]));
			MVector p1(&delaunayOutput.pointlist[currTetra[j]*3]);

			for (int k=0;k<4;k++) {
				if (j!=k){
					//WeightedVertex * pt2 = reinterpret_cast<WeightedVertex *>(static_cast<unsigned>(delaunayOutput.pointattributelist[currTetra[k]]));
					//edges[pt1].insert(pt2);

					MVector p2(&delaunayOutput.pointlist[currTetra[k]*3]);
					MFnNurbsCurve curve;
					MPointArray controlVertices;
					MDoubleArray knotSequences;
					controlVertices.append(p1);
					controlVertices.append(p2);
					knotSequences.append(0);
					knotSequences.append(1);
					curve.create(controlVertices,knotSequences,1,MFnNurbsCurve::kOpen,false,false,curveParent.object());
				}
			}
		}
	}
#endif
#ifdef outputFaces
	tetgenio::facet *currFace = delaunayOutput.facets;
	DEBUG_COUT_ENDL("number of faces "<<delaunayOutput.numberoffacets);
	for (int i=0;i<delaunayOutput.numberoffacets;i++,currFace++){
		
		tetgenio::polygon *currPoly = currFace->polygonlist;
		for (int j=0;j<currFace->numberofpolygons;j++,currPoly++){
			//WeightedVertex * pt1 = reinterpret_cast<WeightedVertex *>(static_cast<unsigned>(delaunayOutput.pointattributelist[currTetra[j]]));
			for (int k=0;k<currPoly->numberofvertices;k++)
				for (int l=0;l<currPoly->numberofvertices;l++)
					if (k!=l){
						MVector p1(&delaunayOutput.pointlist[currPoly->vertexlist[l]*3]);
						MVector p2(&delaunayOutput.pointlist[currPoly->vertexlist[k]*3]);

						MFnNurbsCurve curve;
						MPointArray controlVertices;
						MDoubleArray knotSequences;
						controlVertices.append(p1);
						controlVertices.append(p2);
						knotSequences.append(0);
						knotSequences.append(1);
						curve.create(controlVertices,knotSequences,1,MFnNurbsCurve::kOpen,false,false,curveParent.object());
					}



		}
	}
#endif

#ifdef outputTrifaces
	int *currFace = delaunayOutput.trifacelist;
	DEBUG_COUT_ENDL("number of faces "<<delaunayOutput.numberoftrifaces);
	for (int i=0;i<delaunayOutput.numberoftrifaces;i++,currFace+=3){
			for (int k=0;k<3;k++)
				for (int l=0;l<3;l++)
					if (k!=l){
						MVector p1(&delaunayOutput.pointlist[currFace[l]*3]);
						MVector p2(&delaunayOutput.pointlist[currFace[k]*3]);

						MFnNurbsCurve curve;
						MPointArray controlVertices;
						MDoubleArray knotSequences;
						controlVertices.append(p1);
						controlVertices.append(p2);
						knotSequences.append(0);
						knotSequences.append(1);
						curve.create(controlVertices,knotSequences,1,MFnNurbsCurve::kOpen,false,false,curveParent.object());
					}
	}
#endif


	this->finished();
}
