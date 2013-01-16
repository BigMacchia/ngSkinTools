import unittest
from ngSkinTools.utils import Utils

class UtilsTest(unittest.TestCase):
    
    def testShortName(self):
        self.assertEquals(Utils.shortName("a|b|c"), "c")
        self.assertEquals(Utils.shortName("c"), "c")
        self.assertEquals(Utils.shortName("a:c"), "a:c")
        self.assertEquals(Utils.shortName("a:b|a:c"), "a:c")
        self.assertEquals(Utils.shortName("a:a|a:b|a:c"), "a:c")
        self.assertEquals(Utils.shortName(None), None)
