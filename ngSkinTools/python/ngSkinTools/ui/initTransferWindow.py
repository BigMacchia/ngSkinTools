from ngSkinTools.ui.basetoolwindow import BaseToolWindow
from maya import cmds
from ngSkinTools.ui.basetab import BaseTab
from ngSkinTools.doclink import SkinToolsDocs
from ngSkinTools.ui.events import LayerEvents
from ngSkinTools.ui.uiWrappers import TextLabel, FloatField, StoredTextEdit,\
    DropDownField, FormLayout, CheckBoxField
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.constants import Constants
from ngSkinTools.utils import Utils
from ngSkinTools.ui.SelectHelper import SelectHelper
from ngSkinTools.ui.basedialog import BaseDialog
from ngSkinTools.ui.actions import BaseAction
from ngSkinTools.ui.options import ValueModel
from ngSkinTools.log import LoggerFactory


log = LoggerFactory.getLogger("initTransferWindow")

class AddPairDialog(BaseDialog):
    CTRL_PREFIX = 'ngSkinToolsAddPairDialog_'
    
    def __init__(self):
        BaseDialog.__init__(self)
        self.title = "Add Influences Association"
        self.sourceValue = ValueModel()
        self.destinationValue = ValueModel()
        self.buttons = [self.BUTTON_OK,self.BUTTON_CANCEL]
        
    def updateEnabled(self):
        cmds.layout(self.destinationRow,e=True,enable=not self.chkSelfReference.getValue())
        self.chkBidirectional.setEnabled(not self.chkSelfReference.getValue())
        
    def getAvailableInfluences(self):
        result = []
        layerInfluences = cmds.ngSkinLayer(q=True,listLayerInfluences=True) 
        for index,i in enumerate(layerInfluences):
            if index%2!=0:
                result.append(i)
        return result
        
    def createInnerUi(self,parent):
        
        #container = BaseTab.createScrollLayout(parent)
        rows=cmds.columnLayout(parent=parent,
            adjustableColumn=1,rowSpacing=Constants.MARGIN_SPACING_VERTICAL,
            width=400)
        
        BaseTab.createFixedTitledRow(rows, "Influence names")
        self.chkLongNames = CheckBoxField(AddPairDialog.CTRL_PREFIX+'longNames',label="Longer, where ambiguous",annotation="Show long names in drop down boxes")
        self.chkLongNames.changeCommand.addHandler(self.updateInfluenceLabels)
        
        BaseTab.createFixedTitledRow(rows, "Source influence")
        
        
        self.sourceDropdown = DropDownField(self.sourceValue)

        BaseTab.createFixedTitledRow(rows, None)

        self.chkSelfReference = CheckBoxField(AddPairDialog.CTRL_PREFIX+'selfReference',label="Self reference",annotation="Don't map to another influence, mirror on the same influence")
        self.chkSelfReference.changeCommand.addHandler(self.updateEnabled)
        self.chkBidirectional = CheckBoxField(AddPairDialog.CTRL_PREFIX+'bidirectional',label="Bidirectional association")
            
 
        self.destinationRow = BaseTab.createTitledRow(rows, "Destination influence")
        self.destinationDropdown = DropDownField(self.destinationValue)
        
        self.updateEnabled()
        self.updateInfluenceLabels()
        self.sourceDropdown.updateModel()
        self.destinationDropdown.updateModel()
        return rows
    
    def updateInfluenceLabels(self):
        self.influences = self.getAvailableInfluences()
        self.sourceDropdown.clear()
        self.destinationDropdown.clear()
        
        if self.chkLongNames.getValue():
            nameProcessor = lambda name: cmds.ls(name)[0]
        else:
            nameProcessor = InfluencesListEntry.shortName
        
        for i in self.influences:
            self.sourceDropdown.addOption(nameProcessor(i))
        for i in self.influences:
            self.destinationDropdown.addOption(nameProcessor(i))
            
    def getSourceInfluence(self):
        return self.influences[self.sourceDropdown.model.get()]
    
    def getDestinationInfluence(self):
        return self.influences[self.destinationDropdown.model.get()]
        
class AddPairAction(BaseAction):
    
    def execute(self):
        dlg = AddPairDialog()
        if dlg.execute(parentWindow=self.ownerUI)!=AddPairDialog.BUTTON_OK:
            return
        
        InitTransferWindow.getInstance().content.addManualPair(
                    dlg.getSourceInfluence(),
                    dlg.getDestinationInfluence() if not dlg.chkSelfReference.getModelValue() else dlg.getSourceInfluence(),
                    dlg.chkBidirectional.getModelValue())
        
        
class RemovePairAction(BaseAction):
    def execute(self):
        tab = InitTransferWindow.getInstance().content
        tab.removePairs([i for i in tab.getSelectedPairs() if not i.automatic])
    
    def isEnabled(self):
        for i in InitTransferWindow.getInstance().content.getSelectedPairs():
            if not i.automatic:
                return True
            
        return False;
                
        
class InfluencesListEntry:
    def __init__(self):
        self.source = None
        self.destination = None
        self.bidirectional = False
        self.automatic = True
    
    @staticmethod    
    def shortName(longName):
        separator = longName.rfind("|")
        if separator>=0:
            return longName[separator+1:]
        return longName
    
    def asLabel(self):
        
        if self.source==self.destination:
            result = self.shortName(self.source)+": itself"
        else: 
            mask = "%s -> %s"
            if self.bidirectional:
                mask = "%s <-> %s"
            result = mask % (self.shortName(self.source),self.shortName(self.destination))
                
        if not self.automatic:
            result = "[M] "+result
            
        return result 
    
    def isSelfReference(self):
        return self.source==self.destination
    
    
    def highlight(self):
        SelectHelper.replaceHighlight([self.source,self.destination])
        
    def __repr__(self):
        return self.asLabel()
        

class TransferWeightsTab(BaseTab):
    log = LoggerFactory.getLogger("Transfer Weights Tab")
    VAR_PREFIX = 'ngSkinToolsTransferTab_'
    
    def __init__(self):
        BaseTab.__init__(self)
        self.items = []
        self.manualItems = []
    
    def addManualPair(self,source,destination,bidirectional):
        self.getMll().addManualMirrorInfluenceAssociation(source,destination)
        if bidirectional:
            self.getMll().addManualMirrorInfluenceAssociation(destination,source)
            
        self.execInitMirrorData()
        
    def getMll(self):
        return LayerDataModel.getInstance().mll 

        
    def removePairs(self,pairList):
        changed = False
        for i in pairList:
            changed = True
            
            self.getMll().removeManualMirrorInfluenceAssociation(i.source,i.destination)
            if i.bidirectional:
                self.getMll().removeManualMirrorInfluenceAssociation(i.destination,i.source)
            
        if changed:
            self.execInitMirrorData()
            
    
    def createUI(self, parent):
        # base layout
        self.addPairAction = AddPairAction(self.parentWindow.windowName)
        self.removePairAction = RemovePairAction(parent)
        
        buttons = []
        buttons.append(('Initialize', self.doInitAssociations,''))
        buttons.append(('Close', self.closeWindow,''))
        
        self.cmdLayout = self.createCommandLayout(buttons,SkinToolsDocs.INITWEIGHTTRANSFER_INTERFACE)
        
        self.createInitializationGroup()
        self.createCacheContentsGroup()

        self.updateCacheInfo()
        self.updateLayoutEnabled()
        LayerEvents.layerAvailabilityChanged.addHandler(self.updateLayoutEnabled,self.cmdLayout.outerLayout)
        LayerEvents.mirrorCacheStatusChanged.addHandler(self.updateCacheInfo,self.cmdLayout.outerLayout)
        
        

    def createInitializationGroup(self):
        group = self.createUIGroup(self.cmdLayout.innerLayout, 'Initialization')

        self.createFixedTitledRow(group, 'Infl. Distance Error')
        self.controls.influenceDistanceError = FloatField(self.VAR_PREFIX+'distanceError', minValue=0, maxValue=None, step=0.01, defaultValue=0.001, 
                                    annotation='Defines maximum inaccuracy between left and right influence positions')

        self.createTitledRow(group, 'Influence Prefixes')
        self.controls.influencePrefixes = StoredTextEdit(self.VAR_PREFIX+'inflPrefix', annotation='Defines maximum inaccuracy between left and right influence positions')

        self.createFixedTitledRow(group, 'Mirror Axis')
        cmds.columnLayout()
        self.controls.mirrorAxis = DropDownField(self.VAR_PREFIX+'mirrorAxis')
        self.controls.mirrorAxis.beginRebuildItems()
        self.controls.mirrorAxis.addOption("X");
        self.controls.mirrorAxis.addOption("Y");
        self.controls.mirrorAxis.addOption("Z");
        self.controls.mirrorAxis.endRebuildItems()


    def createCacheContentsGroup(self):
        group = self.createUIGroup(self.cmdLayout.innerLayout, 'Cache Contents')
        self.createTitledRow(group, 'Current status')
        self.controls.labelCacheInfo = TextLabel(align='left')
        self.controls.labelCacheInfo.setBold()
        
        
        influencesLayout = cmds.columnLayout(parent=group,adjustableColumn=1,rowSpacing=Constants.MARGIN_SPACING_VERTICAL)
        self.createTitledRow(influencesLayout, "Influence Associations")
        self.controls.influencesList = cmds.textScrollList(parent=influencesLayout, height=100, numberOfRows=5, allowMultiSelection=True,
                                                           selectCommand=self.onInfluenceSelected,
                                                           deleteKeyCommand=self.removePairAction.execute)
        
        self.controls.influencesMenu = cmds.popupMenu(parent=self.controls.influencesList)
        self.addPairAction.newMenuItem("Add manual association...")
        self.removePairAction.newMenuItem("Remove manual association...")
        
        cmds.setParent(group)
        
    def getSelectedPairs(self):
        selection = cmds.textScrollList(self.controls.influencesList,q=True,sii=True)

        if selection is not None:
            for i in selection:
                yield self.items[i-1]
    
        
    def doInitAssociations(self,*args):
        self.execInitMirrorData()
    
    def closeWindow(self,*args):
        self.parentWindow.closeWindow()


    def execInitMirrorData(self):
        kargs = {};
        kargs["initMirrorData"] = True;
        kargs["influenceAssociationDistance"] = self.controls.influenceDistanceError.getValue()
        kargs["mirrorAxis"] = self.controls.mirrorAxis.getSelectedText()


        # create a comma-delimited prefix string, stripping away any spaces 
        # that might be specified in the user input        
        prefixes = self.controls.influencePrefixes.getValue()
        kargs["influenceAssociationPrefix"] = ",".join([prefix.strip() for prefix in prefixes.split(",")])
        
        cmds.ngSkinLayer(**kargs)
        
        LayerDataModel.getInstance().updateMirrorCacheStatus()

        self.updateInfluenceList()
        
        

    def updateCacheInfo(self):
        '''
        updates UI according to new mirror cache status
        '''
        
        self.controls.labelCacheInfo.setLabel(LayerDataModel.getInstance().mirrorCache.message)
        self.updateInfluenceList()
        
    def onInfluenceSelected(self,*args):
        newHighlight = []
        for i in self.getSelectedPairs():
            newHighlight.append(i.source)
            newHighlight.append(i.destination)
            
        SelectHelper.replaceHighlight(newHighlight)
        
        self.removePairAction.updateEnabled()
        
    @staticmethod
    def findAssociation(list,source,destination,automatic):
        for i in list:
            if i.automatic!=automatic:
                continue
            
            if i.source==source and i.destination==destination:
                return i
            if i.bidirectional and i.destination==source and i.source==destination:
                return i
            
        return None
        
    def updateInfluenceList(self):
        influenceAssociations = cmds.ngSkinLayer(q=True,mirrorCacheInfluences=True)
        if influenceAssociations is None:
            influenceAssociations = []
            
        self.items = []
            
        while len(influenceAssociations)!=0:
            source = influenceAssociations.pop(0)
            destination = influenceAssociations.pop(0)
            automatic = int(influenceAssociations.pop(0))==0
            item=self.findAssociation(self.items, destination, source,automatic)
            if item is not None:
                item.bidirectional = True
            else:
                item = InfluencesListEntry()
                item.source = source
                item.destination = destination
                item.bidirectional = False
                self.items.append(item)
                item.automatic = automatic
                
        def defaultSort(entry1,entry2):
            # priority for non-automatic entries
            if entry1.automatic!=entry2.automatic:
                return 1 if entry1.automatic else -1

            # priority for non-self references
            if entry1.isSelfReference()!=entry2.isSelfReference():
                return 1 if entry1.isSelfReference() else -1
                
            # priority for bidirectional entries
            if entry1.bidirectional!=entry2.bidirectional:
                return 1 if not entry1.bidirectional else -1
            
            return cmp(entry1.source, entry2.source)
                
        
        self.items = sorted(self.items,defaultSort) 
            
            
        newLabels = []
        for i in self.items:
            newLabels.append(i.asLabel())
        
        
        cmds.textScrollList(self.controls.influencesList,e=True,removeAll=True,append=newLabels)
        
        

    def updateLayoutEnabled(self):
        '''
        updates UI enabled/disabled flag based on layer data availability
        '''
        enabled = LayerDataModel.getInstance().layerDataAvailable==True
        cmds.layout(self.cmdLayout.innerLayout,e=True,enable=enabled)
        cmds.layout(self.cmdLayout.buttonForm,e=True,enable=enabled)




class InitTransferWindow(BaseToolWindow):
    def __init__(self,windowName):
        BaseToolWindow.__init__(self,windowName)
        self.windowTitle = 'Init Skin Transfer'
        self.sizeable = True
        self.defaultHeight = 350
        self.defaultWidth = 300
        
    @staticmethod
    def getInstance():
        return BaseToolWindow.getWindowInstance('InitTransferWindow', InitTransferWindow)
        
        
        
    def createWindow(self):
        BaseToolWindow.createWindow(self)
        
        self.content = TransferWeightsTab()
        self.content.parentWindow = self
        self.content.createUI(self.windowName)
        
        
        
        
def test():
    t = InitTransferWindow.getInstance()
    t.showWindow()