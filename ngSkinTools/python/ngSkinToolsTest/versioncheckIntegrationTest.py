from ngSkinToolsTest.decorators import updateCheckServiceRequired, raises

import unittest
from ngSkinTools.versioncheck import VersionChecker, HttpPostTransport,\
    VersionCheckException

class VersionCheckIntegrationTest(unittest.TestCase):
    
    def setUp(self):
        unittest.TestCase.setUp(self)
        
        self.url = "http://localhost:8080/version-check"
        self.checker = VersionChecker()
        self.checker.currentId = "58b89f888667f46baf0b8c8c070dcf233a384ef3"
        self.checker.uniqueClientId = "00111222333"
        
        
        
        self.checker.transport = HttpPostTransport()
        self.checker.transport.host = 'localhost:8080'
        self.checker.transport.path = '/version-check'
    
    @updateCheckServiceRequired
    def testJustWorks(self):
        self.checker.execute()
        
    @updateCheckServiceRequired
    @raises(VersionCheckException)
    def testInvalidPath(self):
        '''
        just to make sure that "testJustWorks" is silent because it works and other mis-config will cause setup to fail
        '''
        self.checker.transport.path = '/check'
        self.checker.execute()
        
