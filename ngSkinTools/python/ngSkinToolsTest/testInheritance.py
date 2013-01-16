'''
Created on 2012.09.10

@author: uiron
'''
import unittest

class A:
    log = "A"
    
    def doA(self):
        return A.log
    
class B(A):
    log = "B"
    
    def doB(self):
        return B.log



class TestInheritance(unittest.TestCase):

    def testDifferent(self):
        '''
        if a class defines a class variable, values are different for parent and child
        '''
        a = A()
        b = B()
        
        self.assertEqual(a.doA(), "A")
        self.assertEqual(b.doA(), "A")
        self.assertEqual(b.doB(), "B")


if __name__ == "__main__":
    #import sys;sys.argv = ['', 'Test.testName']
    unittest.main()