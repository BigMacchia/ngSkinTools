'''
Created on 2012.09.11

@author: uiron
'''
import unittest
from ngSkinToolsTest.mayaremote.mayaClient import MayaClient, MockMayaModule
from ngSkinTools.importExport import LayerData
from ngSkinTools.ui.actions import ExportAction


class Test(unittest.TestCase):


    def setUp(self):
        self.client = MayaClient()
        self.client.connect()
        
        self.cmds = MockMayaModule('cmds')
        self.cmds.client = self.client
        
        self.mel = MockMayaModule('mel')
        self.mel.client = self.client
        
    def assertModelsEqual(self,model1,model2):
        '''
        validates that two layer data models are identical
        '''
        self.assertEquals(len(model1.layers),len(model2.layers))
        
        for l1,l2 in zip(model1.layers,model2.layers):
            self.assertEquals(l1.name, l2.name)
            self.assertEquals(l1.opacity, l2.opacity)
            self.assertEquals(l1.enabled, l2.enabled)
            self.assertEquals(l1.mask, l2.mask)
            
            self.assertEquals(len(l1.influences),len(l2.influences))
            for i1,i2 in zip(l1.influences,l2.influences):
                self.assertEquals(i1.logicalIndex, i2.logicalIndex)
                self.assertEquals(i1.influenceName, i2.influenceName)
                self.assertEquals(i1.weights, i2.weights)
        


    def tearDown(self):
        self.client.disconnect()


    def testLoadModel(self):
        import ngSkinTools.importExport as ie
        ie.cmds = self.cmds
        ie.mel = self.mel
        model = LayerData()
        model.loadFrom('pSphere1')
        print model.layers[0].influences[0].weights
        
        
    def __testExportAction(self):
        file = "testExportActionFile.xml"
        import ngSkinTools.importExport as ie
        import ngSkinTools.utils as u
        ie.cmds = self.cmds
        ie.mel = self.mel
        u.cmds = self.cmds
        
        class ExportActionMock(ExportAction):
            def __init__(self):
                pass
            
            def selectFile(self):
                return file
            
        a = ExportActionMock()
        a.execute()
        


