import unittest
import re
from ngSkinTools.version import Version, generateUniqueClientId
from ngSkinToolsTest.decorators import insideMayaOnly
from maya import cmds
from ngSkinTools.ui.options import Options

class VersionTest(unittest.TestCase):


    @insideMayaOnly
    def testClientIdEqualBetweenCalls(self):
        id1 = Version.uniqueClientId()
        self.assertEqual(id1, Version.uniqueClientId())

    @insideMayaOnly
    def testClientIdDiffersAfterReset(self):
        id1 = Version.uniqueClientId()
        cmds.optionVar(remove=Options.VAR_OPTION_PREFIX+"updateCheckUniqueClientId")
        self.assertNotEqual(id1, Version.uniqueClientId())
        
        
    def testClientIdFormatValid(self):
        '''
        ID must consist of 40 hexadecimal values
        '''
        for _ in xrange(1000):
            id = generateUniqueClientId()
            self.assertTrue(re.match("[a-fA-F0-9]{40}",id),"client id is invalid: "+id)
