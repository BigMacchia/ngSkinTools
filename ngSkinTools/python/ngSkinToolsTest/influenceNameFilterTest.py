import unittest
from ngSkinToolsTest.testUtils import AdditionalAsserts
from ngSkinTools.InfluenceNameFilter import InfluenceNameFilter


class InfluenceNameFilterTest(AdditionalAsserts, unittest.TestCase):
    def testPartialMatch(self):
        f = InfluenceNameFilter().setFilterString("joint")
        self.assertTrue(f.isMatch("joint1"))
        self.assertTrue(f.isMatch("joint2"))
        
    def testMultiMatch(self):
        f = InfluenceNameFilter().setFilterString("joint1 joint2")
        
        self.assertTrue(f.isMatch("joint1"))
        self.assertTrue(f.isMatch("joint2"))
        self.assertFalse(f.isMatch("joint3"))


    def testWildcardMatch(self):
        f = InfluenceNameFilter().setFilterString("joint*_1")
        self.assertTrue(f.isMatch("joint1_1"))
        self.assertTrue(f.isMatch("joint55_1"))
        self.assertTrue(f.isMatch("joint_1"))
        self.assertFalse(f.isMatch("joint1_2"))

    def testFilterOutGarbage(self):
        f = InfluenceNameFilter().setFilterString("joint)({}#$%^@#@!&&&#1")
        self.assertTrue(f.isMatch("joint1"))
        self.assertFalse(f.isMatch("joint2"))
        
    def testCaseInsensitive(self):
        f = InfluenceNameFilter().setFilterString("L_finger4_1 l_finger4_2")
        
        self.assertTrue(f.isMatch("l_finger4_1"))
        self.assertTrue(f.isMatch("L_finger4_2"))
        self.assertFalse(f.isMatch("R_finger4_1"))
