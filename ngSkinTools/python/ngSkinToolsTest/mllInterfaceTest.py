'''
Created on 2012.11.05

@author: uiron
'''

from __future__ import with_statement 
import unittest
from ngSkinTools.mllInterface import MllInterface
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinToolsTest import testUtils
from ngSkinToolsTest.testUtils import AdditionalAsserts
from maya import cmds

class MllInterfaceTest(AdditionalAsserts, unittest.TestCase):
    
    def setUp(self):
        unittest.TestCase.setUp(self)
        self.mll = MllInterface()
        

    @insideMayaOnly
    def testAccessForNoSelection(self):
        testUtils.openMayaFile("normalization.ma")
        cmds.select(cl=True)
        self.mll.setCurrentMesh(None)
        
        self.assertEquals(None,self.mll.getTargetInfo())

    @insideMayaOnly
    def testInvalidInfluenceIndex(self):
        testUtils.openMayaFile("normalization.ma")
        self.mll.setCurrentMesh('testMesh')
        self.mll.initLayers()
        layerId = self.mll.createLayer("initial weights")
        
        weights = [0.0]*self.mll.getVertCount()
        
        with self.assertRaises(errorMessage="skin cluster does not have logical influence 666"):
            self.mll.setInfluenceWeights(layerId, 666, weights)
            
    
    @insideMayaOnly
    def testAccess(self):
        # test the same for specified mesh, or None for current selection
        for mesh in ('testMesh',None):
            testUtils.openMayaFile("normalization.ma")
            self.mll.setCurrentMesh(mesh)
            if mesh is None:
                cmds.select('testMesh')
                            
            self.mll.initLayers()
            layerId = self.mll.createLayer("initial weights")
            self.assertNotEqual(None, layerId)
            
            self.assertEqual("initial weights",self.mll.getLayerName(layerId))
            
            self.assertEqual(16, self.mll.getVertCount(), "Invalid vertex count detected")
            
            self.assertEqual(1, self.mll.getLayerOpacity(layerId))
            self.assertEqual(True, self.mll.isLayerEnabled(layerId))
            self.assertEqual([], self.mll.getLayerMask(layerId))
            self.assertFloatArraysEqual(self.mll.getInfluenceWeights(layerId,1),[0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0])
            
    @insideMayaOnly
    def testOverwriteWeightsCrashesMaya(self):
        testUtils.openMayaFile("genericSkinnedMesh.mb")
        self.mll.setCurrentMesh("mesh|pPlane1")
        self.mll.initLayers()
        layerId = self.mll.createLayer("initial weights",forceEmpty=True)
        weights = [1.0]*self.mll.getVertCount()
        weights[0] = 0.7
        weights[1] = 0.5
        weights[2] = 0.3
        
        self.mll.setInfluenceWeights(layerId, 0, weights)
        self.assertFloatArraysEqual(self.mll.getInfluenceWeights(layerId, 0)[0:3], [0.7,0.5,0.3])

        self.mll.setInfluenceWeights(layerId, 1, weights)
        self.assertFloatArraysEqual(weights,self.mll.getInfluenceWeights(layerId, 1))
        
        # previous influence should have some of it's value substracted
        self.assertFloatArraysEqual(self.mll.getInfluenceWeights(layerId, 0)[0:3], [0.3,0.5,0.3])
        
        
        
    @insideMayaOnly
    def testAddManualMirrorInfluences(self):
        testUtils.openMayaFile("genericSkinnedMesh.mb")
        self.mll.setCurrentMesh("mesh|pPlane1")
        self.mll.initLayers()
        layerId = self.mll.createLayer("initial weights")
        
        result = self.mll.listManualMirrorInfluenceAssociations()
        self.assertArraysEqual(result, [])
        
        self.mll.addManualMirrorInfluenceAssociation("L_joint1", "R_joint1")
        self.mll.addManualMirrorInfluenceAssociation("R_joint1", "L_joint1")
        
        result = self.mll.listManualMirrorInfluenceAssociations()
        self.assertDictionariesEqual(result, {"L_joint1":"R_joint1","R_joint1":"L_joint1"})
        
        
        