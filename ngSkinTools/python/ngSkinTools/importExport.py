'''
Example export:

.. code-block:: python

    data = LayerData()
    data.loadFrom('skinnedMesh')
    exporter = XmlExporter()
    xml = exporter.process(data)
    saveXmlToFile(xml,'path/to/my.xml') # write contents to file here

Example import:

.. code-block:: python

    importer = XmlImporter()
    xml = loadFileData('path/to/my.xml') # read file contents here
    data = importer.process(xml)
    data.saveTo('skinnedMesh')

'''
from __future__ import with_statement
from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.utils import Utils, MessageException
from maya import cmds
from maya import OpenMaya as om
from maya import OpenMayaAnim as oma
from ngSkinTools import utils
from ngSkinTools.skinClusterFn import SkinClusterFn

class Influence(object):
    '''
    Single influence in skin layer data.
    '''
    
    def __init__(self):
        
        # influence logical index in a skin cluster
        self.logicalIndex = -1
        
        # full path of the influence in the scene
        self.influenceName = None
        
        # influence weights for each vertex (list of double)
        self.weights = []
        
    def __repr__(self):
        return "[Infl %r]" % (self.influenceName)
        
        


class Layer(object):
    '''
    Represents single layer; can contain any amount of influences.
    '''
    
    def __init__(self):
        # layer name
        self.name = None
        
        # layer opacity
        self.opacity = 0.0
        
        # layer on/off flag
        self.enabled = False
        
        # list of influences in this layer with their weights (list of Influence)
        self.influences = []
        
        # layer mask (could be None or list of double)
        self.mask = None
        
    def addInfluence(self, influence):
        '''
        Register :class:`Influence` for given layer
        '''
        
        assert isinstance(influence, Influence)
        self.influences.append(influence)
        
    def __repr__(self):
        return "[Layer %r %r %r %r]" % (self.name, self.opacity, self.enabled, self.influences)
    
        
        
        
class LayerData(object):
    '''
    Intermediate data object between ngSkinTools core and importers/exporters,
    representing all layers info in one skin cluster. 
    '''
    
    def __init__(self):
        #: layers list
        self.layers = []
        self.mll = MllInterface()

    def addLayer(self, layer):
        '''
        register new layer into this data object
        '''
        assert isinstance(layer, Layer)
        self.layers.append(layer)
       
    @staticmethod 
    def getFullNodePath(nodeName):
        result = cmds.ls(nodeName,l=True)
        if result is None or len(result)==0:
            raise MessageException("node %s was not found" % nodeName)
        
        return result[0]
        
    def loadFrom(self, mesh):
        '''
        loads data from actual skin cluster and prepares it for exporting.
        supply skin cluster or skinned mesh as an argument
        '''
        
        self.mll.setCurrentMesh(mesh)
        
        for layerID, layerName in self.mll.listLayers():
            layer = Layer()
            layer.name = layerName
            self.addLayer(layer)
            
            
            layer.opacity = self.mll.getLayerOpacity(layerID)
            layer.enabled = self.mll.isLayerEnabled(layerID)
            
            layer.mask = self.mll.getLayerMask(layerID)
            
            for inflName, logicalIndex in self.mll.listLayerInfluences(layerID):
                influence = Influence()
                influence.influenceName = self.getFullNodePath(inflName)
                influence.logicalIndex = logicalIndex
                layer.addInfluence(influence)
                
                influence.weights = self.mll.getInfluenceWeights(layerID, logicalIndex)
                
    def __validate(self):
        numVerts = self.mll.getVertCount()
        for layer in reversed(self.layers):
            maskLen = len(layer.mask)
            if maskLen != 0 and maskLen != numVerts:
                raise Exception("Invalid vertex count for mask in layer '%s': expected size is %d" % (layer.name, numVerts))
            
            for influence in layer.influences:
                weightsLen = len(influence.weights) 
                if weightsLen != numVerts:
                    raise Exception("Invalid weights count for influence '%s' in layer '%s': expected size is %d" % (influence.influenceName, layer.name, numVerts))
                
                influence.logicalIndex = self.skinClusterFn.getLogicalInfluenceIndex(influence.influenceName)
                
        
    @Utils.undoable        
    def saveTo(self, mesh):
        '''
        saves data to actual skin cluster
        '''
        
        # set target to whatever was provided
        self.mll.setCurrentMesh(mesh)
        
        
        if not self.mll.getLayersAvailable():
            self.mll.initLayers()
            
        if not self.mll.getLayersAvailable():
            raise Exception("could not initialize layers")
        

        mesh, self.skinCluster = self.mll.getTargetInfo()
        self.skinClusterFn = SkinClusterFn()
        self.skinClusterFn.setSkinCluster(self.skinCluster)
        
        self.__validate()
        
        # set target to actual mesh
        self.mll.setCurrentMesh(mesh)
            
        with self.mll.batchUpdateContext():
            for layer in reversed(self.layers):
                layerId = self.mll.createLayer(name=layer.name, forceEmpty=True)
                self.mll.setCurrentLayer(layerId)
                if layerId is None:
                    raise Exception("import failed: could not create layer '%s'" % (layer.name))
                
                self.mll.setLayerOpacity(layerId, layer.opacity)
                self.mll.setLayerEnabled(layerId, layer.enabled)
                self.mll.setLayerMask(layerId, layer.mask)
                
                for influence in layer.influences:
                    self.mll.setInfluenceWeights(layerId, influence.logicalIndex, influence.weights)
        
                
    def __repr__(self):
        return "[LayerDataModel(%r)]" % self.layers
    
    def getAllInfluences(self):
        '''
        a convenience method to retrieve a list of names of all influences used in this layer data object
        '''
        
        result = set()
        
        for layer in self.layers:
            for influence in layer.influences:
                result.add(influence.influenceName)
                
        return tuple(result)
    
    
class XmlExporter:
    def __init__(self):
        from xml.dom import minidom
        
        self.document = minidom.Document()
        self.baseElement = None
        self.layerElement = None
        self.influenceElement = None
        
    def processInfluence(self, influence):
        self.influenceElement = self.document.createElement("influence")
        self.layerElement.appendChild(self.influenceElement)
        
        self.influenceElement.setAttribute("index", str(influence.logicalIndex))
        self.influenceElement.setAttribute("name", str(influence.influenceName))
        self.floatArrayToAttribute(self.influenceElement, "weights", influence.weights)
        
    def processLayer(self, layer):
        self.layerElement = self.document.createElement("layer")
        self.baseElement.appendChild(self.layerElement)
        
        self.layerElement.setAttribute("name", str(layer.name))
        self.layerElement.setAttribute("enabled", "yes" if layer.enabled else "no")
        self.layerElement.setAttribute("opacity", self.formatFloat(layer.opacity))
        self.floatArrayToAttribute(self.layerElement, "mask", layer.mask)
        
        for influence in layer.influences:
            self.processInfluence(influence)
    
    def process(self, layerDataModel):
        '''
        transforms LayerDataModel to UTF-8 xml
        '''
        self.baseElement = self.document.createElement("ngstLayerData")
        self.baseElement.setAttribute("version", "1.0")
        self.document.appendChild(self.baseElement)
        
        for layer in layerDataModel.layers:
            self.processLayer(layer)
            
        return self.document.toprettyxml(encoding="UTF-8")
            
    def floatArrayToAttribute(self, node, attrName, values):
        node.setAttribute(attrName, " ".join(map(self.formatFloat, values)))
            
    def formatFloat(self, value):
        # up to 15 digits after comma, no trailing zeros if present
        return "%.15g" % value
    
    
class XmlImporter:
    
    def iterateChildren(self, node, name):
        for i in node.childNodes:
            if i.nodeName == name:
                yield i
                
    def attributeToFloatList(self, node, attribute):
        value = node.getAttribute(attribute).strip()
        if len(value) == 0:
            return []
        
        return map(float, value.split(" "))
    
    def process(self, xml):
        'transforms XML to LayerDataModel'
        self.model = LayerData()
        
        from xml.dom import minidom
        self.dom = minidom.parseString(xml)
        
        for layersNode in self.iterateChildren(self.dom, "ngstLayerData"):
            for layerNode in self.iterateChildren(layersNode, "layer"):
                layer = Layer()
                self.model.addLayer(layer)
                layer.name = layerNode.getAttribute("name")
                layer.enabled = layerNode.getAttribute("enabled") in ["yes", "true", "1"]
                layer.opacity = float(layerNode.getAttribute("opacity"))
                layer.mask = self.attributeToFloatList(layerNode, "mask")
                
                for influenceNode in self.iterateChildren(layerNode, "influence"):
                    influence = Influence()
                    influence.influenceName = influenceNode.getAttribute("name")
                    influence.logicalIndex = int(influenceNode.getAttribute("index"))
                    influence.weights = self.attributeToFloatList(influenceNode, "weights")
                    layer.addInfluence(influence)
                
        
        return self.model
        
class JsonExporter:
    
    def __influenceToDictionary(self, influence):
        result = {}
        result['name'] = influence.influenceName
        result['index'] = influence.logicalIndex
        result['weights'] = influence.weights
        return result
    
    def __layerToDictionary(self, layer):
        result = {}
        result['name'] = layer.name
        result['opacity'] = layer.opacity
        result['enabled'] = layer.enabled
        result['mask'] = layer.mask
        result['influences'] = []
        for infl in layer.influences:
            result['influences'].append(self.__influenceToDictionary(infl))
            
        return result
    
    def __modelToDictionary(self, model):
        result = {}
        result['layers'] = []
        for layer in model.layers:
            result['layers'].append(self.__layerToDictionary(layer))
            
        return result
    
    def process(self, layerDataModel):
        '''
        transforms LayerDataModel to JSON
        '''
        import json
        return json.dumps(self.__modelToDictionary(layerDataModel))
    
    
class JsonImporter:
    def process(self, jsonDocument):
        '''
        transform JSON document into layerDataModel
        '''
        import json
        self.document = json.loads(jsonDocument)
        
        model = LayerData()
        for layerData in self.document["layers"]:
            layer = Layer()
            model.addLayer(layer)        
            layer.enabled = layerData['enabled']
            layer.mask = layerData['mask']
            layer.name = layerData['name']
            layer.opacity = layerData['opacity']
            layer.influences = []

            for influenceData in layerData['influences']:
                influence = Influence()
                layer.addInfluence(influence)
                influence.weights = influenceData['weights']
                influence.logicalIndex = influenceData['index']
                influence.influenceName = influenceData['name']

        return model
    
class Format:
    def __init__(self):
        self.title = ""
        self.exporterClass = None
        self.importerClass = None
        self.recommendedExtensions = ()    
    
class Formats:
    
    
    @staticmethod
    def getXmlFormat():
        f = Format()
        f.title = "XML"
        f.exporterClass = XmlExporter
        f.importerClass = XmlImporter
        f.recommendedExtensions = ("xml",)
        return f
    
    @staticmethod
    def getJsonFormat():
        f = Format()
        f.title = "JSON"
        f.exporterClass = JsonExporter
        f.importerClass = JsonImporter
        f.recommendedExtensions = ("json", "txt")
        return f
    
    @staticmethod
    def getFormats():
        '''
        returns iterator to available exporters
        '''
        yield Formats.getXmlFormat()
        if Utils.getMayaVersion() > Utils.MAYA2010:
            yield Formats.getJsonFormat()
        
        
    
        
        

