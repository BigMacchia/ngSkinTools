def launchTestsDelayed():
    from ngSkinTools import log as logModule
    logModule.LoggerFactory = logModule.SimpleLoggerFactory()
    
    from ngSkinToolsTest.runInMaya import storeTestResult
    from ngSkinToolsTest.runInMaya import runAllTests
    runAllTests()
    storeTestResult()

# run tests when maya becomes idle    
from maya import cmds
cmds.scriptJob(runOnce=True,e=["idle", launchTestsDelayed]);