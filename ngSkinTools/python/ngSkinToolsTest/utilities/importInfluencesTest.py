from __future__ import with_statement
import unittest
from ngSkinToolsTest.testUtils import AdditionalAsserts
from ngSkinTools.utilities.importInfluences import ImportInfluences
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinToolsTest import testUtils
from ngSkinTools.utils import MessageException, Utils
from maya import cmds
from ngSkinTools.ui.utilities.importInfluencesUi import ImportInfluencesAction
from ngSkinTools.skinClusterFn import SkinClusterFn
from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.log import LoggerFactory

log = LoggerFactory.getLogger("ImportInfluencesTest")

class ImportInfluencesTest(AdditionalAsserts, unittest.TestCase):
    
    def setUp(self):
        unittest.TestCase.setUp(self)
        
        self.importer = ImportInfluences()
        
        testUtils.openMayaFile("influence transfer.mb")
    
    @insideMayaOnly
    def testDetectSkinClusters(self):
        self.importer.setSourceFromMesh("sourceMesh")
        self.assertEqual(self.importer.sourceSkinCluster, "skinCluster1")
        
        self.importer.setDestinationFromMesh("destinationMesh")
        self.assertEqual(self.importer.destinationSkinCluster, "skinCluster2")
        
    @insideMayaOnly
    def testWrongMesh(self):
        with self.assertRaises(MessageException, "cannot find skin cluster attached"):
            self.importer.setSourceFromMesh("joint1")
    
    
    @insideMayaOnly
    def testListInfluences(self):
        self.importer.setSourceFromMesh("sourceMesh")
        
        influences = self.importer.listInfluences(self.importer.sourceSkinCluster)
        self.assertArraysEqual(influences, ["|joint1","|joint1|joint2|joint3","|joint1|joint2|joint3|joint4|joint5"])
        
    @insideMayaOnly
    def testListInfluencesDiff(self):
        self.importer.setSourceFromMesh("sourceMesh")
        self.importer.setDestinationFromMesh("destinationMesh")
        
        influences = self.importer.listInfluencesDiff()
        self.assertArraysEqual(influences,  ["|joint1","|joint1|joint2|joint3"])
        
    @insideMayaOnly
    def testAddInfluence(self):
        self.importer.setSourceFromMesh("sourceMesh")
        self.importer.setDestinationFromMesh("destinationMesh")
        self.importer.addInfluence("joint3")

        influences = self.importer.listInfluencesDiff()
        self.assertArraysEqual(influences,  ["|joint1"])
        
        skinCluster = SkinClusterFn()
        skinCluster.setSkinCluster(self.importer.destinationSkinCluster)
        influenceLogicalIndex = skinCluster.getLogicalInfluenceIndex("joint3")
        
        mll = MllInterface()
        mll.setCurrentMesh("destinationMesh")
        mll.initLayers()
        layerId = mll.createLayer("test")
        weights = mll.getInfluenceWeights(layerId, influenceLogicalIndex)
        print weights
        self.assertFloatArraysEqual(weights, [0.0]*25)
        
        
    @insideMayaOnly
    def testInitFromSelection(self):
        cmds.select("sourceMesh")
        cmds.select("destinationMesh",add=True)
        self.importer.initFromSelection()
        self.assertEqual(self.importer.sourceSkinCluster, "skinCluster1")
        self.assertEqual(self.importer.destinationSkinCluster, "skinCluster2")

    @insideMayaOnly
    def testInitFromWrongSelection(self):
        '''
        various scenarios of response to invalid selection
        '''
        
        cmds.select(clear=True)
        with self.assertRaises(MessageException, "select two skinned meshes"):
            self.importer.initFromSelection()

        cmds.select("sourceMesh","destinationMesh","joint1")
        with self.assertRaises(MessageException, "select two skinned meshes"):
            self.importer.initFromSelection()

        cmds.select("sourceMesh")
        with self.assertRaises(MessageException, "select two skinned meshes"):
            self.importer.initFromSelection()

        cmds.select("joint1")
        with self.assertRaises(MessageException, "select two skinned meshes"):
            self.importer.initFromSelection()
            
        cmds.select("joint1","joint2")
        with self.assertRaises(MessageException, "cannot find skin cluster attached to joint1"):
            self.importer.initFromSelection()


    @insideMayaOnly
    def testImportAction(self):
        print "skipping testImportAction"
        return
    
        cmds.select("sourceMesh")
        cmds.select("destinationMesh",add=True)
        
        action = ImportInfluencesAction(ownerUI=None)
        action.execute()
