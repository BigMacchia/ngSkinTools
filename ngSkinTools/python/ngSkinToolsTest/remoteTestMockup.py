import unittest
from ngSkinToolsTest.mayaremote.mayaClient import MayaClient, MockMayaModule


class TestMayaSocket(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        self.client = MayaClient()
        unittest.TestCase.__init__(self, methodName=methodName)

    def setUp(self):
        self.client.connect()

    def tearDown(self):
        self.client.disconnect()

    def testSimpleCommand(self):
        print self.client.executeCommand('cmds.ls',"persp",l=True)
        
    def testQueryLayers(self):
        result = self.client.executeCommand('cmds.ngSkinLayer','pSphere1',q=True,listLayers=True)
        print repr(result)
        
    def testMockCmds(self):
        cmds = MockMayaModule('cmds')
        cmds.client = self.client
        result = cmds.ngSkinLayer('pSphere1',q=True,listLayers=True)
        print result


