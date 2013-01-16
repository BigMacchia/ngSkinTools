import unittest
from maya import cmds,utils as mUtils
from ngSkinTools.ui.mainwindow import MainWindow
from ngSkinToolsTest.decorators import insideMayaOnly
from ngSkinTools.ui.tabMirror import TabMirror
from ngSkinToolsTest.testUtils import openMayaFile, closeNextDialogWithResult,\
    runInNextModalDialog
from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.ui.basedialog import BaseDialog
from ngSkinTools.log import LoggerFactory

log = LoggerFactory.getLogger("MainWindowTest")

class MainWindowTest(unittest.TestCase):
    
    def setUp(self):
        MainWindow.closeAllWindows()

    def tearDown(self):
        MainWindow.closeAllWindows()
    
    @insideMayaOnly
    def testOpenClose(self):
        MainWindow.open()
        MainWindow.closeAllWindows()

    @insideMayaOnly
    def testOpenWithInvalidOptions(self):
        MainWindow.closeAllWindows()
        cmds.optionVar(stringValue=['ngSkinToolsMirrorTab_mirrorDirection','x'])
        MainWindow.open()
        
        
        
    @insideMayaOnly
    def testAddManualInfluence(self):
        openMayaFile('simplemirror.ma')
        cmds.select("testMeshY")
        mll = MllInterface()
        mll.initLayers()
        mll.setCurrentLayer(mll.createLayer("test layer"))
        
        
        window = MainWindow.open()
        mirrorTab = window.findTab(TabMirror)
        
        initWindow = mirrorTab.execInitMirror()
        
        def selectPairAndClickOk(dialog):
            log.info("running inside modal dialog "+str(dialog))
            dialog.sourceDropdown.setValue(2)
            dialog.destinationDropdown.setValue(3)
            dialog.chkSelfReference.setValue(False)
            #dialog.closeDialogWithResult(BaseDialog.BUTTON_OK)
            closeNextDialogWithResult(BaseDialog.BUTTON_OK)
            
        log.info("modal dialog setup")
        runInNextModalDialog(selectPairAndClickOk)
        log.info("modal dialog setup ended")
            
        initWindow.content.addPairAction.execute()
        
        self.assertEquals(str(initWindow.content.items[0]),"[M] L_joint2 <-> L_joint3", "manual pair addition failed")
        
        
        
        
