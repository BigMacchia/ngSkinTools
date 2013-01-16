import unittest
from ngSkinToolsTest import testUtils
from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.ui.actions import ExportAction, ImportAction, ImportOptions,\
    BaseAction
from maya import cmds
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinTools.importExport import Formats
from ngSkinTools.ui.headlessDataHost import HeadlessDataHost
import os
import tempfile
import random
from ngSkinToolsTest.testUtils import openMayaFile

class BaseActionTest(unittest.TestCase):
    
    def testUiCmd(self):
        class TestAction(BaseAction):
            pass
        a = TestAction(None)
        cmd = a.createUiCommand()
        self.assertTrue("%r" % cmd, "Action_TestAction") 
        self.assertTrue("%s" % cmd, "Action_TestAction") 
        self.assertTrue(str(cmd), "Action_TestAction") 
        self.assertTrue(unicode(cmd), "Action_TestAction") 

class ActionsTest(unittest.TestCase):
    def setUp(self):
        unittest.TestCase.setUp(self)
        testUtils.openMayaFile("normalization.ma")
        self.mll = mll = MllInterface()
        mll.setCurrentMesh("testMesh")
        mll.initLayers()
        mll.createLayer("initial weights")
        mll.createLayer("second layer")
        
        HeadlessDataHost.HANDLE.addReference(self)

                
        self.exportName = os.path.join(tempfile.gettempdir(), 'layersTestData_%d.xml' %random.randint(0,1024*1024))
        self.assertTrue(not os.path.exists(self.exportName), 'target file already exists')
        
        
    def tearDown(self):
        if (os.path.exists(self.exportName)):
            os.unlink(self.exportName)
        
        
        unittest.TestCase.tearDown(self)
        HeadlessDataHost.HANDLE.removeReference(self)


    def getFileContents(self,fileName):
        f = open(fileName)
        try:
            contents = "".join(f.readlines())
        finally:
            f.close()
        return contents
    
    def writeFile(self,fileName,fileContents):
        f = open(fileName,'w')
        try:
            f.write(fileContents)
        finally:
            f.close()

    @insideMayaOnly
    def testExport(self):
        cmds.select('testMesh')

        # create export file name        
        
        a = ExportAction(ownerUI=None, ioFormat=Formats.getXmlFormat())
        a.selectFile = lambda *args,**kwargs: self.exportName
        a.execute()
        
        contents = self.getFileContents(self.exportName)
        
        # simple contents validation just to check that we've written our data correctly
        self.assertTrue(contents.find('name="second layer"')>=0)
            
            
    @insideMayaOnly
    def testImport(self):
        exampleData = """<?xml version="1.0" encoding="UTF-8"?>
                            <ngstLayerData version="1.0">
                                <layer enabled="yes" mask="" name="imported layer 2" opacity="1"/>
                                <layer enabled="yes" mask="" name="imported layer 1" opacity="1">
                                    <influence index="0" name="joint1" weights="1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0"/>
                                    <influence index="1" name="joint2" weights="0 2.13095271869166e-16 2.13584887749867e-16 0 0 0 0 0 1 1 1 1 1 1 1 1"/>
                                    <influence index="2" name="joint3" weights="0 8.94933305586526e-18 8.45971717509877e-18 0 0 0 0 0 0 0 0 0 0 0 0 0"/>
                                </layer>
                            </ngstLayerData>
                        """
                        
           
        self.writeFile(self.exportName, exampleData)
        
        cmds.select('testMesh')
        
        a = ImportAction(ownerUI=None, ioFormat=Formats.getXmlFormat())
        a.selectFile = lambda *args,**kwargs: self.exportName
        a.getImportOptions = lambda : ImportOptions()
        a.execute()

        # assert that first two layers are imported ones, in a correct order        
        layers = list(self.mll.listLayers())
        self.assertEquals(layers[0][1],'imported layer 2')
        self.assertEquals(layers[1][1],'imported layer 1')
        
    @insideMayaOnly
    def testImportAvailableWithNoLayers(self):
        exampleData = """<?xml version="1.0" encoding="UTF-8"?>
                            <ngstLayerData version="1.0">
                                <layer enabled="yes" mask="" name="imported layer 1" opacity="1">
                                    <influence index="0" name="joint1" weights="1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0"/>
                                </layer>
                            </ngstLayerData>
                        """
        self.writeFile(self.exportName, exampleData)

        openMayaFile('normalization.ma')

        a = ImportAction(ownerUI=None, ioFormat=Formats.getXmlFormat())
        a.selectFile = lambda *args,**kwargs: self.exportName
        a.getImportOptions = lambda : ImportOptions()

        cmds.select('persp')
        self.assertFalse(a.isEnabled(), "import action should be disabled if target mesh is not suitable for skinning")

        cmds.select('testMesh')
        self.assertTrue(a.isEnabled(), "import action should be available if suitable mesh is selected")

        # should allow execution here
        a.execute()
        
        
    @insideMayaOnly
    def testCleanExistingLayers(self):
        exampleData = """<?xml version="1.0" encoding="UTF-8"?>
                            <ngstLayerData version="1.0">
                                <layer enabled="yes" mask="" name="imported layer 1" opacity="1">
                                    <influence index="0" name="joint1" weights="1 1 1 1 1 1 1 1 0 0 0 0 0 0 0 0"/>
                                </layer>
                            </ngstLayerData>
                        """
        self.writeFile(self.exportName, exampleData)

        openMayaFile('normalization.ma')
        cmds.select('testMesh')
        self.mll.setCurrentMesh('testMesh')
        self.mll.initLayers()
        self.mll.createLayer("one")
        self.mll.createLayer("two")
        
        self.assertEquals(len(list(self.mll.listLayers())),2)
        
        options = ImportOptions()
        options.keepExistingLayers = False
        
        a = ImportAction(ownerUI=None, ioFormat=Formats.getXmlFormat())
        a.selectFile = lambda *args,**kwargs: self.exportName
        a.getImportOptions = lambda: options

        a.execute()
        
        newLayers = list(self.mll.listLayers())
        
        self.assertEquals(len(newLayers),1)
        layerId = newLayers[0][0]
        influences = list(self.mll.listLayerInfluences(layerId))
        
        self.assertEqual(len(influences), 1)
                
        
        

