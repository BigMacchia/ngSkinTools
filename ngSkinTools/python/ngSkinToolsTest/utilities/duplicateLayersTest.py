from __future__ import with_statement
import unittest
from ngSkinToolsTest.testUtils import AdditionalAsserts
from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.log import LoggerFactory
from ngSkinTools.utilities.duplicateLayers import DuplicateLayers
from ngSkinToolsTest import testUtils
from ngSkinToolsTest.decorators import insideMayaOnly

log = LoggerFactory.getLogger("ImportInfluencesTest")

class DuplicateLayersTest(AdditionalAsserts, unittest.TestCase):
    def setUp(self):
        unittest.TestCase.setUp(self)
        
        self.mll = MllInterface()
        
        self.setup = DuplicateLayers()
        self.setup.setMllInterface(self.mll)
        
        testUtils.openMayaFile("influence transfer.mb")
        
        self.mll.setCurrentMesh('sourceMesh')
        
        self.mll.initLayers()
        
    def testLayerName(self):
        self.assertEquals("layer1 copy",self.setup.createLayerName("layer1"))
        self.assertEquals("layer1 copy(2)",self.setup.createLayerName("layer1 copy"))
        self.assertEquals("layer1 copy(1000001)",self.setup.createLayerName("layer1 copy(1000000)"))
        
        
    @insideMayaOnly
    def testDuplicateOneLayer(self):
        id = self.mll.createLayer("layer1 copy")
        self.setup.addLayer(id)
        self.setup.execute()
        
        newId = self.setup.duplicateIds[0]
        self.assertEqual(self.mll.getLayerName(newId),'layer1 copy(2)')
        self.assertEqual(self.mll.isLayerEnabled(newId),True)
        self.assertAlmostEqual(self.mll.getLayerOpacity(newId),1.0)
        self.assertFloatArraysEqual(self.mll.getLayerMask(newId), [])
        
        self.assertFloatArraysEqual(self.mll.getInfluenceWeights(id, 1), self.mll.getInfluenceWeights(newId, 1))
    
    @insideMayaOnly
    def testLayerOrder(self):
        id1 = self.mll.createLayer("layer1")
        id2 = self.mll.createLayer("layer2")
        
        def layerNames():
            return [a[1] for a in self.mll.listLayers()]
    
        self.assertArraysEqual(layerNames(), ["layer2","layer1"])
        
        self.setup.addLayer(id1)
        self.setup.addLayer(id2)
        
        self.setup.execute()
        
        # order respects "newer layers on top"
        self.assertArraysEqual(layerNames(), ["layer2 copy","layer1 copy","layer2","layer1"])
        
        # individual indexes in copy array respect indexes in the original array
        self.assertEquals(self.mll.getLayerName(self.setup.duplicateIds[0]),"layer1 copy")
        self.assertEquals(self.mll.getLayerName(self.setup.duplicateIds[1]),"layer2 copy")
        
    