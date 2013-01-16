from __future__ import with_statement
import unittest
from ngSkinToolsTest import allTests
from ngSkinTools.utils import Utils
from ngSkinTools.log import LoggerFactory

log = LoggerFactory.getLogger("mayaTestRunner")

lastTestResult = "Tests were not run"

def runSuite(suite):
    global lastTestResult
    lastTestResult = "Tests are being executed"
    try:
        Utils.DEBUG_MODE = True
        Utils.loadPlugin()
        result = unittest.TextTestRunner(verbosity=2).run(suite)
        
        lastTestResult = "tests executed: %d, tests failed: %d" % (result.testsRun,len(result.errors)+len(result.failures))
        
        for i in result.errors+result.failures:
            lastTestResult += "\n"
            lastTestResult += "FAILED: "+str(i[0])
    except:
        lastTestResult = "Tests failed to finish"
        raise
        
    print lastTestResult 
    
    return result

def runTestClass(suiteClass):
    testSuite = unittest.TestSuite()
    testSuite.addTest(unittest.makeSuite(suiteClass))
    return runSuite(testSuite)
    
def runSingleTest(suiteClass,testName):
    testSuite = unittest.TestSuite()
    testSuite.addTest(suiteClass(testName))
    return runSuite(testSuite)
    
def runAllTests():
    return runSuite(unittest.TestLoader().loadTestsFromModule(allTests))


def storeTestResult():
    '''
    stores test result in a file, using a name in environment variable MAYA_TEST_RESULT_FILE
    '''
    import os
    fileName = os.getenv("MAYA_TEST_RESULT_FILE", default = None)
    if fileName is None:
        return
    
    global lastTestResult
    log.info(lastTestResult)
    with open(fileName, "w") as f:
        f.write(lastTestResult)
    
    
def runTests():
    return runSingleTest(allTests.MainWindowTest,'testAddManualInfluence')
    
    #runTestClass(allTests.MllInterfaceTest)
    #runTestClass(VariousImportScenarios)
    #runTestClass(MllInterfaceTest)
    #runAllTests()
    

