'''
Toolbar buttons for launching from Maya
'''


def runUnitTests():
    from ngSkinTools.debug import reloader
    reload(reloader)
    reloader.unload()
    
    from ngSkinTools import log as logModule
    logModule.LoggerFactory = logModule.SimpleLoggerFactory()
    
    logger = logModule.LoggerFactory.getLogger("VersionChecker")
    
    logger.info("starting tests...")
    from ngSkinToolsTest.runInMaya import runTests
    runTests()
    logger.info("done")
    
def runAllTests():
    '''
    .. code-block:: python

        from ngSkinToolsTest import debugToolbarButtons
        reload(debugToolbarButtons)
        debugToolbarButtons.runAllTests()    
        
    '''
    
    from ngSkinTools.debug import reloader
    reload(reloader)
    reloader.unload()
    
    from ngSkinTools import log as logModule
    logModule.LoggerFactory = logModule.SimpleLoggerFactory()
    
    logger = logModule.LoggerFactory.getLogger("VersionChecker")
    
    logger.info("starting tests...")
    from ngSkinToolsTest.runInMaya import runAllTests
    runAllTests()
    logger.info("done")    



def openMainWindow():
    '''
    opens main window. shelf setup with reload:

    .. code-block:: python

        from ngSkinTools.debug import reloader
        reload(reloader)
        reloader.unload()
        from ngSkinToolsTest import debugToolbarButtons
        debugToolbarButtons.openMainWindow()
    
    '''
    from ngSkinTools import log
    log.LoggerFactory = log.SimpleLoggerFactory()
    
    from ngSkinTools.context import applicationContext
    from ngSkinToolsTest.debugApplicationSetup import DebugApplicationSetup
    applicationContext.setup = DebugApplicationSetup()
    
    
    from ngSkinTools.utils import Utils
    Utils.DEBUG_MODE = True
    from ngSkinTools.ui.mainwindow import MainWindow
    MainWindow.open()
    
def getUserMayaFolder(mayaVersion,osPlatform):
    '''
    returns maya settings folder in user's home dir
    '''
    import os
    from platform import system
    homedir = os.getenv('USERPROFILE') or os.getenv('HOME')
    platformnames = {64:'x64',32:'win32'}
    mayaFolder = "%d-%s" % (mayaVersion,platformnames[osPlatform])
    
    if system() in ('Windows','Microsoft'):
        homedir = os.path.join(homedir,'My Documents')
    
    return os.path.join(homedir,"maya",mayaFolder)    
    
def loadPluginFromBuildTab():
    from ngSkinTools.debug import reloadplugin
    from ngSkinTools.utils import Utils
    from os import path
    import os
     
     
    mayaVersion = Utils.getMayaVersion()
    
    pluginPath = path.join(path.dirname(path.dirname(path.dirname(__file__))),'build-target','windows','maya%d-64bit'%mayaVersion,'plugin','ngSkinTools.mll')
    targetDir = path.join(getUserMayaFolder(mayaVersion, 64),"plug-ins")
    if not os.path.exists(targetDir):
        os.makedirs(targetDir)
        
    targetPath = path.join(targetDir,"ngSkinTools.mll")
    print "copying plugin from '%s' to '%s'" % (pluginPath,targetPath) 
    
    reloadplugin.reload(pluginPath,targetPath)
    
    
def removePythonModulePath():
    import sys
    for i in sys.path[:]:
        print i
        if "ngSkinTools" in i:
            sys.path.remove(i)
            
    for i in sys.modules.keys():
        if "ngSkinTools" in i:
            del(sys.modules[i])
            