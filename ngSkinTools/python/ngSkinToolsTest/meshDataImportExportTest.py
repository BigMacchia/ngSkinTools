from maya import cmds,OpenMaya as om
import maya.OpenMayaMPx as OpenMayaMPx
import unittest
from ngSkinToolsTest.testUtils import AdditionalAsserts, openMayaFile
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinTools.utils import Utils, MessageException

  
class MeshDataExporter:
    
    def __init__(self):
        self.meshMObject = None
        
    def useSkinClusterInputMesh(self,skinCluster):
        '''
        sets skincluster's input mesh as source mesh
        '''
        skinClusterObject = Utils.getMObjectForNode(skinCluster)
        plug = om.MPlug(skinClusterObject,OpenMayaMPx.cvar.MPxDeformerNode_inputGeom)
        plug.selectAncestorLogicalIndex(0,OpenMayaMPx.cvar.MPxDeformerNode_input)
        self.meshMObject = plug.asMObject()
        
    def exportMeshTriangles(self,meshTransform):
        '''
        returns mesh triangles: first vertex list, then vertex ID list for each triangle;
        meshTransform (supplied as transform node name) is required to transform
        each vertex to world-space
        '''

        # get world transform matrix for given mesh transform node        
        meshTransformNodeFn = om.MFnDependencyNode(Utils.getMObjectForNode(meshTransform))
        matrix = meshTransformNodeFn.findPlug("worldMatrix")
        transform = om.MFnMatrixData(matrix.elementByLogicalIndex(0).asMObject()).matrix()

        # get triangles for the mesh
        fnMesh = om.MFnMesh(self.meshMObject)
        counts = om.MIntArray()
        vertices = om.MIntArray()
        fnMesh.getTriangles(counts,vertices)
        idList = [id for id in Utils.mIter(vertices)]
        
        # get point values
        points = om.MPointArray()
        fnMesh.getPoints(points)
        pointList = []
        for p in Utils.mIter(points):
            p = p*transform
            pointList.extend((p.x,p.y,p.z))

        # return point values, id values            
        return pointList,idList 
        
           

class MeshDataImportExportTest(AdditionalAsserts, unittest.TestCase):
    
    
    def createMeshData(self):
        meshParent = om.MFnMeshData()
        parent = meshParent.create()
        
        
        mesh = om.MFnMesh()
        verts = om.MPointArray()
        verts.append(om.MPoint(0,0,0))
        verts.append(om.MPoint(1,0,0))
        verts.append(om.MPoint(0,1,0))
        
        polyCounts = om.MIntArray()
        polyCounts.append(3)
        
        polyConnects = om.MIntArray()
        polyConnects.append(0)
        polyConnects.append(1)
        polyConnects.append(2)
        
        mesh.create(3, # num verts
                        1, # num polys
                        verts,
                        polyCounts,
                        polyConnects,
                        parent
                        )
        
        del(meshParent)
        
    @insideMayaOnly
    def testExportData(self):
        openMayaFile("meshExport.mb")

        exporter = MeshDataExporter()
        exporter.useSkinClusterInputMesh("skinCluster1")
        
        verts,ids = exporter.exportMeshTriangles("pPlane1")
        
        # check first vertice: must be in world coords
        self.assertAlmostEqual(verts[0], 0.378,places=3)
        self.assertAlmostEqual(verts[1], 0.954,places=3)
        self.assertAlmostEqual(verts[2], 0.011,places=3)
        
        
        
    