from __future__ import with_statement
import unittest
from ngSkinToolsTest import testUtils
from ngSkinTools.mllInterface import MllInterface
from maya import cmds
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinToolsTest.testUtils import AdditionalAsserts
from ngSkinTools.layerUtils import LayerUtils
from ngSkinTools.utilities.weightsClipboard import WeightsClipboard
from ngSkinTools.utils import MessageException

class CopyPasteActionsTest(AdditionalAsserts, unittest.TestCase):
    def setUp(self):
        testUtils.openMayaFile("normalization.ma")
        
        self.mll = MllInterface()
        cmds.select("testMesh")
        self.mll.initLayers()
        self.mll.createLayer("initial weights")
        self.layer = self.mll.createLayer("second layer")
        self.mll.setCurrentLayer(self.layer)
        
        self.clipboard = WeightsClipboard(self.mll)
        
        self.newWeights = [0.5]*self.mll.getVertCount()
        
    def tearDown(self):
        unittest.TestCase.tearDown(self)


    @insideMayaOnly
    def testCantCopyEmptyMask(self):
        with self.assertRaises(MessageException, "Nothing copied"):
            self.mll.setCurrentPaintTarget(LayerUtils.PAINT_TARGET_MASK)
            self.clipboard.withCurrentLayerAndInfluence().copy()

        # shold not be able to paste after this
        with self.assertRaises(MessageException, "Nothing to paste"):
            self.clipboard.withCurrentLayerAndInfluence().paste(True)
            

    @insideMayaOnly
    def testCantPasteIfNothingIsThere(self):
        with self.assertRaises(MessageException, "Nothing to paste"):
            self.clipboard.withCurrentLayerAndInfluence().paste(True)
        

    @insideMayaOnly
    def testCopyMask(self):
        # set mask to something
        self.mll.setLayerMask(self.layer,self.newWeights)
        self.assertArraysEqual(self.mll.getLayerMask(self.layer), self.newWeights)

        self.mll.setCurrentPaintTarget(LayerUtils.PAINT_TARGET_MASK)
        self.clipboard.withCurrentLayerAndInfluence().copy()
        
        self.assertArraysEqual(self.clipboard.copiedWeights, self.newWeights)
        
    @insideMayaOnly
    def testPasteMaskReplace(self):
        self.mll.setLayerMask(self.layer,self.newWeights)
        self.mll.setCurrentPaintTarget(LayerUtils.PAINT_TARGET_MASK)
        self.clipboard.copiedWeights = self.newWeights
        self.clipboard.withCurrentLayerAndInfluence().paste(True)
        self.assertArraysEqual(self.mll.getLayerMask(self.layer), self.newWeights)
        
    @insideMayaOnly
    def testPasteInfluenceReplace(self):
        influence = 1
        self.mll.setInfluenceWeights(self.layer, influence, self.newWeights)
        self.mll.setCurrentPaintTarget(influence)
        self.clipboard.copiedWeights = self.newWeights
        self.clipboard.withCurrentLayerAndInfluence().paste(True)
        self.assertArraysEqual(self.mll.getInfluenceWeights(self.layer,influence), self.newWeights)
        
