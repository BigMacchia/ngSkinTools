from maya import cmds
from ngSkinTools.version import Version
from ngSkinTools.ui.tabSkinRelax import TabSkinRelax
from ngSkinTools.ui.tabAssignWeights import TabAssignWeights
from ngSkinTools.ui.updateCheckWindow import UpdateCheckWindow
from ngSkinTools.utils import Utils
from ngSkinTools.ui.targetDataDisplay import TargetDataDisplay
from ngSkinTools.ui.uiWrappers import FormLayout
from ngSkinTools.ui.constants import Constants
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.tabMirror import TabMirror
from ngSkinTools.ui.tabPaint import TabPaint
from ngSkinTools.ui.dlgAbout import AboutDialog
from ngSkinTools.ui.actions import InfluenceFilterAction, NewLayerAction,\
    DeleteLayerAction, MoveLayerAction, LayerPropertiesAction,\
    ConvertMaskToTransparencyAction, ConvertTransparencyToMaskAction,\
    MirrorLayerWeightsAction, BaseAction, RemovePreferencesAction,\
    EnableDisableLayerAction, ExportAction, ImportAction
from ngSkinTools.ui.basetoolwindow import BaseToolWindow
from ngSkinTools.log import LoggerFactory
from ngSkinTools.importExport import Formats
from ngSkinTools.ui.utilities.importInfluencesUi import ImportInfluencesAction
from ngSkinTools.utilities.duplicateLayers import DuplicateLayers
from ngSkinTools.ui.utilities.duplicateLayerAction import DuplicateLayersAction
from ngSkinTools.ui.headlessDataHost import HeadlessDataHost
from ngSkinTools.doclink import SkinToolsDocs
from ngSkinTools.ui.utilities.weightsClipboardActions import CopyWeights,\
    CutWeights, PasteWeightsAdd, PasteWeightsReplace
from ngSkinTools.layerUtils import LayerUtils
from ngSkinTools.ui.tabSettings import TabSettings

log = LoggerFactory.getLogger("MainWindow")


class MainMenu:
    def __init__(self):
        pass
    
    def execCheckForUpdates(self,*args):
        UpdateCheckWindow.execute(silent=False)
        
    @Utils.undoable
    def execCleanNodes(self,*args):
        if not LayerUtils.hasCustomNodes():
            Utils.confirmDialog(icon='information', title='Info', message='Scene does not contain any custom ngSkinTools nodes.', button=['Ok']);
            return
        
        message = 'This command deletes all custom nodes from ngSkinTools plugin. Skin weights will be preserved, but all layer data will be lost. Do you want to continue?'
        if Utils.confirmDialog(
                icon='warning',
                title='Warning', 
                message=message, 
                button=['Yes','No'], defaultButton='No')!='Yes':
            return
        
        LayerDataModel.getInstance().cleanCustomNodes()
        
    def execClose(self,*args):
        MainWindow.getInstance().closeWindow()
        
    def execAbout(self,*args):
        AboutDialog().execute()
        
    def createExportSubmenu(self):
        cmds.menuItem(label='Export',subMenu=True,mnemonic='x' )
        for action in MainWindow.getInstance().actions.export:
            action.newMenuItem("As "+action.ioFormat.title+" ...")
        cmds.setParent("..",menu=True)
        
    def createImportSubmenu(self):
        cmds.menuItem(label='Import',subMenu=True,mnemonic='m' )
        for action in MainWindow.getInstance().actions.importActions:
            action.newMenuItem("From "+action.ioFormat.title+" ...")
        cmds.setParent("..",menu=True)
        
    def createLayersMenu(self,actions):
        cmds.menu( label='Layers',mnemonic='L' )
        actions.newLayer.newMenuItem("New Layer...")
        actions.duplicateLayer.newMenuItem("Duplicate Selected Layer(s)")
        actions.deleteLayer.newMenuItem("Delete Selected Layer(s)")
        self.createDivider()
        actions.moveLayerUp.newMenuItem("Move Current Layer Up")
        actions.moveLayerDown.newMenuItem("Move Current Layer Down")

        # import/export        
        self.createDivider()
        self.createExportSubmenu()
        self.createImportSubmenu()
        
        self.createDivider()
        actions.layerProperties.newMenuItem("Properties...")
        self.createDivider()
        cmds.menuItem( label='Close',mnemonic='C',command=self.execClose )
        
    def createEditMenu(self,actions):
        cmds.menu( label='Edit',mnemonic='E' )
        actions.copyWeights.newMenuItem('Copy Influence Weights')
        actions.cutWeights.newMenuItem('Cut Influence Weights')
        actions.pasteWeightsAdd.newMenuItem('Paste Weights (Add)')
        actions.pasteWeightsReplace.newMenuItem('Paste Weights (Replace)')
        self.createDivider()
        actions.convertMaskToTransparency.newMenuItem('Convert Mask to Transparency')
        actions.convertTransparencyToMask.newMenuItem('Convert Transparency to Mask')
        self.createDivider()
        cmds.menuItem( label='Delete Custom Nodes',command=self.execCleanNodes)
        actions.removePreferences.newMenuItem('Reset to Default Preferences')

    def createToolsMenu(self,actions):
        cmds.menu( label='Tools',mnemonic='T' )
        actions.importInfluences.newMenuItem("Import Influences...")
        
    def viewManual(self,*args):
        documentation = HeadlessDataHost.get().documentation
        documentation.openLink(SkinToolsDocs.DOCUMENTATION_ROOT)

    def createHelpMenu(self,actions):
        cmds.menu( label='Help',mnemonic='H' )
        cmds.menuItem( label='View Manual Online',command=self.viewManual )
        cmds.menuItem( label='Check for Updates',command=self.execCheckForUpdates )
        self.createDivider()
        cmds.menuItem( label='About ngSkinTools',mnemonic='A',command=self.execAbout )
        
    def createDivider(self):
        cmds.menuItem( divider=True)
        
        
    def create(self):
        actions = MainWindow.getInstance().actions
        
        self.createLayersMenu(actions)
        self.createEditMenu(actions)
        self.createToolsMenu(actions)
        self.createHelpMenu(actions)



        
    
class MainUiActions:
    def __init__(self,ownerUI):
        self.influenceFilter = InfluenceFilterAction(ownerUI) 
        self.newLayer = NewLayerAction(ownerUI)
        self.deleteLayer = DeleteLayerAction(ownerUI)
        self.moveLayerUp = MoveLayerAction(True,ownerUI)
        self.moveLayerDown = MoveLayerAction(False,ownerUI)
        self.layerProperties = LayerPropertiesAction(ownerUI)
        self.removePreferences = RemovePreferencesAction(ownerUI)
        
        self.convertMaskToTransparency = ConvertMaskToTransparencyAction(ownerUI)
        self.convertTransparencyToMask = ConvertTransparencyToMaskAction(ownerUI)
        
        self.mirrorWeights = MirrorLayerWeightsAction(ownerUI)
        self.enableDisableLayer = EnableDisableLayerAction(ownerUI)
        
        self.importInfluences = ImportInfluencesAction(ownerUI)
        
        self.duplicateLayer = DuplicateLayersAction(ownerUI)
        
        self.copyWeights = CopyWeights(ownerUI)
        self.cutWeights = CutWeights(ownerUI)
        self.pasteWeightsAdd = PasteWeightsAdd(ownerUI)
        self.pasteWeightsReplace = PasteWeightsReplace(ownerUI)
        

                
        self.export = []
        self.importActions = []
        for f in Formats.getFormats():
            self.export.append(ExportAction(ownerUI,ioFormat=f))
            self.importActions.append(ImportAction(ownerUI,ioFormat=f))
        
    def updateEnabledAll(self):
        '''
        updates all actions hosted in this instance
        '''
        
        # update all field-based actions
        for i in dir(self):
            field = getattr(self,i)
            if isinstance(field,BaseAction):
                field.updateEnabled()
                
        # update all action lists
        for i in self.export+self.importActions:
            i.updateEnabled()
        

class MainWindow(BaseToolWindow):
    WINDOW_NAME = 'ngSkinToolsMainWindow'
    WINDOW_DEFAULT_WIDTH = 400;
    WINDOW_DEFAULT_HEIGHT = 500;
    
    @staticmethod
    @Utils.visualErrorHandling
    def open():
        '''
        just a shortcut method to construct and display main window
        '''

        window = MainWindow.getInstance()
        window.showWindow()
        
        # don't know where to fit this in, it's just an utility warning for those trying to run
        # this on a different maya version
        if Utils.getMayaVersion()==Utils.MAYAUNSUPPORTEDVERSION:
            Utils.displayError('unsupported Maya version detected.')
            
        Utils.silentCheckForUpdates()
        
        return window
        
    @staticmethod
    def getInstance():
        '''
        returns instance of a main window; returned value is only valid while window is opened.
        '''
        
        return BaseToolWindow.getWindowInstance(MainWindow.WINDOW_NAME,MainWindow)
        
        
    def __init__(self,windowName):
        log.debug("creating main window")
        
        BaseToolWindow.__init__(self,windowName)
        
        self.windowTitle = self.createWindowTitle()
        
        self.mainTabLayout = None
        self.tabs = []
        
        
        # layer target UI - compound for layers list/no layer data ui
        self.targetUI = None
        
        self.actions = None
        
        self.defaultWidth = MainWindow.WINDOW_DEFAULT_WIDTH
        self.defaultHeight = MainWindow.WINDOW_DEFAULT_HEIGHT
        
        self.sizeable = True


    def createWindowTitle(self):
        '''
        creates main window title
        '''
        return Version.getReleaseName()
    
    def createWindow(self):
        '''
            creates main GUI window and it's contents
        '''
        
        BaseToolWindow.createWindow(self)
        
        self.targetUI = TargetDataDisplay()
        self.actions = MainUiActions(self.windowName)
        
        self.mainMenu = MainMenu()
        self.mainMenu.create();
        
        
        

        # putting tabs in a from targetUiLayout is needed to workaround maya2011 
        # bug with an additional empty tab appearing otherwise
        form = FormLayout(parent=self.windowName)
        
        targetUiLayout = self.targetUI.create(form)
        form.attachForm(targetUiLayout, 0, Constants.MARGIN_SPACING_HORIZONTAL,None,Constants.MARGIN_SPACING_HORIZONTAL)
        
        self.mainTabLayout = cmds.tabLayout(childResizable=True,parent=form,scrollable=False,innerMarginWidth=3)
        form.attachControl(self.mainTabLayout, targetUiLayout, Constants.MARGIN_SPACING_VERTICAL, None,None,None)
        form.attachForm(self.mainTabLayout, None, 0,0,0)
        
        self.tabPaint = self.addTab(TabPaint())
        self.tabMirror = self.addTab(TabMirror())
        self.tabRelax = self.addTab(TabSkinRelax())
        self.tabAssignWeights = self.addTab(TabAssignWeights())
        self.tabSettings = self.addTab(TabSettings())
        

        self.actions.updateEnabledAll()
        
        
        
    def addTab(self,tab):
        '''
        adds tab object to tab UI, creating it's ui and attaching to main window
        '''
        cmds.setParent(self.mainTabLayout)
        layout = tab.createUI(self.mainTabLayout)
        cmds.tabLayout( self.mainTabLayout, edit=True, tabLabel=((layout, tab.getTitle())));
        tab.parentWindow = self
        self.tabs.append(tab)
        
        return tab
        

        
    def findTab(self,tabClass):
        for i in self.tabs:
            if isinstance(i,tabClass):
                return i
            
        return None    