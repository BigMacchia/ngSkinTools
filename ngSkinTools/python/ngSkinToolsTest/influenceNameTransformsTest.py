import unittest
from ngSkinTools.InfluenceNameTransforms import InfluenceNameTransform
from ngSkinToolsTest.testUtils import openMayaFile, AdditionalAsserts
from ngSkinToolsTest.decorators import insideMayaOnly

class InfluenceNameTransformsTest(AdditionalAsserts, unittest.TestCase):

    def testCommonPrefix(self):
        result = InfluenceNameTransform().transform(['joint2','joint1|joint2|joint3','joint3|joint4'])
        self.assertArraysEqual(result, ["joint2","joint3","joint4"])

        names = ['a|b|c1','a|b|c2','a|b|c3','a|b|c3|c4']
        newNames = InfluenceNameTransform().appendingOriginalName().transform(names)
        self.assertArraysEqual(newNames, ["c1","c2","c3","c4"])

        names = ['a|b|c1','a|b|c2','a|b|c2|c4','a|b|c3|c4']
        newNames = InfluenceNameTransform().appendingOriginalName().transform(names)
        self.assertArraysEqual(newNames, ["c1","c2","c4 (c2|c4)","c4 (c3|c4)"])

        names = ['a|b|c1','a|b|c2','a|b|c2|c4','a|b|c3|c4']
        newNames = InfluenceNameTransform().transform(names)
        self.assertArraysEqual(newNames, ["c1","c2","c2|c4","c3|c4"])
