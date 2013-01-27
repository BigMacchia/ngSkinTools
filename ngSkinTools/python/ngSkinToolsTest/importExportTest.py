from __future__ import with_statement 
import unittest
import ngSkinTools.importExport as ie
from ngSkinToolsTest import decorators
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinTools.utils import Utils, MessageException
from ngSkinTools import log as logModule
from ngSkinToolsTest.testUtils import openMayaFile, AdditionalAsserts
from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.layerUtils import LayerUtils
try:
    from flexmock import flexmock
except:
    pass

from maya import cmds

from ngSkinTools.importExport import XmlExporter, XmlImporter, JsonExporter,\
    JsonImporter, LayerData, Layer, Influence


class LayerDataTest(AdditionalAsserts, unittest.TestCase):
    def testInfluenceList(self):
        model = LayerData()

        layer = Layer()
        model.addLayer(layer)

        influence = Influence()
        influence.influenceName = "infl1"
        layer.addInfluence(influence)

        influence = Influence()
        influence.influenceName = "infl2"
        layer.addInfluence(influence)

        influence = Influence()
        influence.influenceName = "infl3"
        layer.addInfluence(influence)

        layer = Layer()
        model.addLayer(layer)

        influence = Influence()
        influence.influenceName = "infl4"
        layer.addInfluence(influence)

        influence = Influence()
        influence.influenceName = "infl3"
        layer.addInfluence(influence)
        
        allInfluences = model.getAllInfluences()
        
        self.assertEquals(len(allInfluences), 4, "four influences should be added")
        
        for i in ["infl1","infl2","infl3","infl4"]:
            assert i in allInfluences

        

class ImportExportTest(AdditionalAsserts, unittest.TestCase):
    log = logModule.LoggerFactory.getLogger("ImportExportTest")
    
    def testInvalidLayerObject(self):
        model = ie.LayerData()
        with self.assertRaises(AssertionError):
            model.addLayer("whatever")
        
    def createSampleModel(self):
        '''
        creates sample LayerData to be used as quick starting point in tests
        '''
        
        model = ie.LayerData()
        
        layer = ie.Layer()
        layer.opacity = 0.00001
        layer.enabled = False
        layer.mask = [1.0,1.0,0,0]
        layer.name = "base  layer"
        model.addLayer(layer)

        layer = ie.Layer()
        model.addLayer(layer)
        layer.opacity = 0.9000000000009
        layer.enabled = True
        layer.mask = [1.0,1.0,0,0]
        layer.name = "layer 2"
        
        infl = ie.Influence()
        layer.addInfluence(infl)
        infl.influenceName = "root|L_Joint1"
        infl.logicalIndex = 0
        infl.weights = [0.1,0.2,0.3,0.4]
        
        return model
        
        
    def testCreation(self):
        self.createSampleModel()
       
        
    def testXml(self):
        '''
        test serialize to XML and back
        '''
        model1 = self.createSampleModel()
        exporter = XmlExporter()
        xml = exporter.process(model1)
        
        importer = XmlImporter()
        model2 = importer.process(xml)
        
        # make sure unserialized model matches original model
        self.assertModelsEqual(model1, model2)
        
    
    @decorators.requiresDependency('json') 
    def testJson(self):
        model1 = self.createSampleModel()
        exporter = JsonExporter()
        json = exporter.process(model1)
        
        importer = JsonImporter()
        model2 = importer.process(json)
        
        self.assertModelsEqual(model1, model2)
        
        
    @decorators.requiresDependency('flexmock') 
    def testLoad(self):
        skinCluster = 'skinCluster1'
        
        mll = flexmock()
        mll.should_receive('setCurrentMesh').once()
        mll.should_receive('listLayers').and_return(((123,"layer1"),(456,"layer2"))).once()
        mll.should_receive('getLayerOpacity').with_args(123).and_return(0.9)
        mll.should_receive('getLayerOpacity').with_args(456).and_return(1.0)
        mll.should_receive('isLayerEnabled').with_args(123).and_return(True)
        mll.should_receive('isLayerEnabled').with_args(456).and_return(False)
        mll.should_receive('getLayerMask').with_args(123).and_return([1.2,1.1,1.0,0])
        mll.should_receive('getLayerMask').with_args(456).and_return([1.2,1.1,1.0,0])
        mll.should_receive('listLayerInfluences').with_args(123).and_return((('infl1',0),('infl2',1)))
        mll.should_receive('listLayerInfluences').with_args(456).and_return((('infl3',0),('infl4',1)))
        mll.should_receive('getInfluenceWeights').with_args(123,0).and_return([0,0,0,0])
        mll.should_receive('getInfluenceWeights').with_args(123,1).and_return([0,0,0,0])
        mll.should_receive('getInfluenceWeights').with_args(456,0).and_return([0,0,0,0])
        mll.should_receive('getInfluenceWeights').with_args(456,1).and_return([0,0,0,0])
        
        model = LayerData()
        model.getFullNodePath = lambda a: a
        model.mll = mll
        model.loadFrom(skinCluster)
        
        self.assertEquals(model.layers[0].name, "layer1")
        self.assertEquals(model.layers[1].name, "layer2")
        self.assertEquals(model.layers[0].opacity, 0.9)
        self.assertEquals(model.layers[0].enabled, True)
        self.assertEquals(model.layers[1].enabled, False)

        self.assertEquals(model.layers[1].influences[0].influenceName, "infl3")
        self.assertEquals(model.layers[1].influences[1].influenceName, "infl4")
        
        self.assertEquals(model.layers[0].influences[0].logicalIndex, 0)
        self.assertEquals(model.layers[0].influences[1].logicalIndex, 1)
        
    @insideMayaOnly
    def testNodePath(self):
        model = LayerData()
        self.assertEquals(model.getFullNodePath("persp"),"|persp")
        
    @insideMayaOnly
    def testLoad2(self):
        openMayaFile('simplemirror.ma')
        mll = MllInterface()
        mll.setCurrentMesh('testMesh')
        mll.initLayers()
        mll.createLayer("Initial Weights")
        
        model = LayerData()
        model.loadFrom('testMesh')
        
        self.assertEqual(model.layers[0].name, "Initial Weights")
        self.assertEquals(model.layers[0].influences[0].influenceName, "|x_axis|root")
        


        
        
        

class VariousImportScenarios(AdditionalAsserts, unittest.TestCase):
    
    def setUp(self):
        self.targetMesh = 'testMesh' 
        self.numVerts = 169 # number of verts in our mesh
        self.mll = MllInterface()
        
        unittest.TestCase.setUp(self)
        openMayaFile('simplemirror.ma')
        cmds.hide("|y_axis")
        cmds.showHidden("|x_axis")
    
        self.mll.setCurrentMesh(self.targetMesh)
        self.mll.initLayers()
        self.mll.createLayer("original weights")
        
        self.model = ie.LayerData()
        self.layer = ie.Layer()
        self.layer.opacity = 1.0
        self.layer.enabled = True
        self.layer.mask = [0.0]*self.numVerts
        self.layer.mask[1] = 0.9 
        self.layer.mask[5] = 0.5566 
        self.layer.name = "imported layer"
        self.model.addLayer(self.layer)

        self.infl = ie.Influence()
        self.layer.addInfluence(self.infl)
        self.infl.influenceName = "x_axis|root|R_joint1"
        self.infl.logicalIndex = 666 # use nonsense value
        self.infl.weights = [0.1]*self.numVerts
        self.infl.weights[3] = 0.688
        self.infl.weights[4] = 0.345
        
        
    @decorators.insideMayaOnly
    def testSave(self):
        self.model.saveTo(self.targetMesh)
        
        layers = list(self.mll.listLayers())
        id,name = layers[0]
        self.assertEquals("imported layer", name)
        self.assertFloatArraysEqual(self.mll.getLayerMask(id), self.layer.mask)
        self.assertFloatArraysEqual(self.mll.getInfluenceWeights(id, self.infl.logicalIndex), self.infl.weights)
        
    @decorators.insideMayaOnly
    def testEmptyMask(self):
        self.layer.mask = []
        self.model.saveTo(self.targetMesh)
        

    @decorators.insideMayaOnly
    def testInvalidVertCount(self):
        self.infl.weights = self.infl.weights[:-1]
        with self.assertRaises(Exception,"Invalid weights count for influence 'x_axis|root|R_joint1' in layer 'imported layer': expected size is 169"):
            self.model.saveTo(self.targetMesh)
            
    @decorators.insideMayaOnly
    def testInvalidMaskCount(self):
        self.layer.mask = self.layer.mask[:-1]
        with self.assertRaises(Exception,"Invalid vertex count for mask in layer 'imported layer': expected size is 169"):
            self.model.saveTo(self.targetMesh)

    @decorators.insideMayaOnly
    def testInvalidMesh(self):
        with self.assertRaises(Exception,"could not initialize layers"):
            self.model.saveTo("persp")

    @decorators.insideMayaOnly
    def testUninitializedLayers(self):
        '''
        if layers on mesh were uninitialized, just initialize layers as usual, and do not fail
        '''
        LayerUtils.deleteCustomNodes()
        
        self.model.saveTo(self.targetMesh)

    @decorators.insideMayaOnly
    def testMissingInfluence(self):
        self.infl.influenceName = "something_is_missing_here"
        
        with self.assertRaises(MessageException,"Could not find influence 'something_is_missing_here' in skinCluster1"):
            self.model.saveTo(self.targetMesh)
            
            


