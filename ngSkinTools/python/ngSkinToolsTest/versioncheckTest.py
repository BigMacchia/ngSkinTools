import unittest
from ngSkinTools.versioncheck import VersionChecker, CheckerTransport,\
    VersionCheckException
import re



class DummyTransport(CheckerTransport):
    '''
    dummy transport to use for pushing in the response
    '''
    def __init__(self):
        self.response = None
    
    def getResponseFor(self, request):
        return self.response


class VersionCheckTest(unittest.TestCase):
    UPDATE_AVAILABLE = """<?xml version="1.0" ?>
    <version-information updateAvailable="yes">
        <current-version>
            <maya-release>2011</maya-release>
            <platform>64</platform>
            <version>1.x</version>
            <os>windows</os>
            <line>stable</line>
            <title>1.1beta</title>
            <release-date>2011-12-30</release-date>
            <uuid>123456789012345</uuid>
        </current-version>
        <links>
            <link url="http://www.neglostyti.com/yousuck/">home page</link>
            <link url="http://www.neglostyti.com/yousuck/downloads.php#platform=windows">download version 1.1beta</link>
        </links>
    </version-information>
    """
    UPDATE_AVAILABLE_NO_LINKS = """<?xml version="1.0" ?>
    <version-information updateAvailable="yes">
        <current-version>
            <maya-release>2011</maya-release>
            <platform>64</platform>
            <version>1.x</version>
            <os>windows</os>
            <line>stable</line>
            <title>1.1beta</title>
            <release-date>2011-12-30</release-date>
            <uuid>123456789012345</uuid>
        </current-version>
    </version-information>
    """

        
    def runWithResponse(self,response):
        self.checker = VersionChecker()
        self.checker.currentId="1234567890123456"
        
        self.checker.transport = DummyTransport()
        self.checker.transport.response = response
        self.checker.execute()

    def testUpdateAvailable(self):
        self.runWithResponse(self.UPDATE_AVAILABLE)
        
        self.assertTrue(self.checker.updateAvailable, "Update should be available")
        self.assertEqual(self.checker.updateTitle, "1.1beta", "Update title is '1.1beta'")
        self.assertEqual(self.checker.updateDate, "2011-12-30", "Update title is '2011-12-30'")
        
        
        self.assertEqual(len(self.checker.getLinks()), 2, "Link count should be 2")
        
        self.assertEqual(self.checker.getLinks()[0].title,"home page", "First link title is 'home page'")
        self.assertEqual(self.checker.getLinks()[0].url,"http://www.neglostyti.com/yousuck/", "First link url is 'http://www.neglostyti.com/yousuck/'")

    def testUpdateNoLinks(self):
        self.runWithResponse(self.UPDATE_AVAILABLE_NO_LINKS)
        
    
    def runWithoutTag(self,tag):
        self.runWithResponse(re.sub('<%s>[^<]*</%s>'%(tag,tag),'',self.UPDATE_AVAILABLE))

    def runWithEmptyTag(self,tag):
        self.runWithResponse(re.sub('<%s>[^<]*</%s>'%(tag,tag),'<%s />'%tag,self.UPDATE_AVAILABLE))


    def runWithoutAttr(self,attr):
        self.runWithResponse(re.sub('%s\s*=\s*"[^"]*"'%(attr),'',self.UPDATE_AVAILABLE))
        
    def runWithEmptyAttr(self,attr):
        self.runWithResponse(re.sub('%s\s*=\s*"[^"]*"'%(attr),'%s=""'%attr,self.UPDATE_AVAILABLE))
        
    def testDate(self):
        self.assertRaises(VersionCheckException,self.runWithoutTag,'release-date')
        self.assertRaises(VersionCheckException,self.runWithEmptyTag,'release-date')
    
    def testMissingTitle(self):
        self.assertRaises(VersionCheckException,self.runWithoutTag,'title')
        self.assertRaises(VersionCheckException,self.runWithEmptyTag,'title')
        
        
    def testMissingUpdateAttr(self):
        self.assertRaises(VersionCheckException,self.runWithoutAttr,'updateAvailable')
        self.assertRaises(VersionCheckException,self.runWithEmptyAttr,'updateAvailable')
        
    def testMissingUrlAttr(self):
        self.assertRaises(VersionCheckException,self.runWithoutAttr,'url')
        self.assertRaises(VersionCheckException,self.runWithEmptyAttr,'url')
        
        
    def testEmptyValues(self):
        self.assertRaises(VersionCheckException, self.runWithResponse,self.UPDATE_AVAILABLE.replace("home page", " "))
        self.assertRaises(VersionCheckException, self.runWithResponse,self.UPDATE_AVAILABLE.replace("release-date", " "))
        self.assertRaises(VersionCheckException, self.runWithResponse,self.UPDATE_AVAILABLE.replace("yes", " "))
        
    def testInvalidResponseStructure(self):
        self.assertRaises(VersionCheckException, self.runWithResponse,"<invalid-structure/>")
        
        
    
            
        
