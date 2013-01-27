from maya import cmds,OpenMaya as om
import unittest
from ngSkinToolsTest.testUtils import AdditionalAsserts, openMayaFile
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinTools.meshDataExporter import MeshDataExporter

  
class MeshDataExporterTest(AdditionalAsserts, unittest.TestCase):
    
    
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
        self.assertAlmostEqual(verts[0], 6.189,places=3)
        self.assertAlmostEqual(verts[1], -0.667,places=3)
        self.assertAlmostEqual(verts[2], 0.402,places=3)
        
        
        
    