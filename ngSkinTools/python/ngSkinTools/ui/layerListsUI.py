from maya import cmds
from ngSkinTools.ui.uiWrappers import FormLayout, TextEdit, RadioButtonField
from ngSkinTools.ui.constants import Constants
from ngSkinTools.layerUtils import LayerUtils
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.events import LayerEvents, Signal, MayaEvents
from ngSkinTools.utils import Utils
from ngSkinTools.ui.options import Options, PersistentValueModel
from ngSkinTools.log import LoggerFactory
from ngSkinTools.InfluenceNameTransforms import InfluenceNameTransform
from ngSkinTools.InfluenceNameFilter import InfluenceNameFilter
from ngSkinTools.ui.basetab import BaseTab

log = LoggerFactory.getLogger("layerListsUI")


class IdToNameEntry:
    def __init__(self,id=None,name=None,displayName=None,suffix=None):
        self.id = id
        self.name = name
        self.displayName = displayName if displayName is not None else name
        self.suffix = suffix
        
    def __repr__(self):
        return "IdToNameEntry(%d:%s,%s)" % (self.id,self.name,self.suffix)

    
class TreeViewIDList:
    def __init__(self,allowMultiSelection=True):
        self.items = []
        selectCommand = self.selectionChanged
        editCommand = self.editLabelCommand
        
        if Utils.getMayaVersion()<Utils.MAYA2012:
            selectCommand = Utils.createMelProcedure(self.selectionChanged, [('int','item'),('int','state')],returnType="int")
            editCommand = Utils.createMelProcedure(self.editLabelCommand, [('string','item'),('string','newName')])
            
        self.control = cmds.treeView(numberOfButtons=0, height=100, selectCommand=selectCommand, editLabelCommand=editCommand)
        
        if Utils.getMayaVersion()>=Utils.MAYA2011:
            cmds.treeView(self.control,e=True,enableKeys=True)
        
        self.selectedID = None
        self.selectedItems = set()
        self.onSelectionChanged = Signal()
        
        self.__selectionChanging = False
        self.__itemNameChanging = False

    def setItems(self,newItems,selectedID=None):
        if selectedID is None:
            selectedID = self.getSelectedID()
            
        if self.items!=newItems:
            self.doSetItems(newItems)
            
        self.items = newItems
        self.selectByID(selectedID)

    def selectionChanged(self,item,selected):
        self.__selectionChanging = True
        try:
            selected = int(selected)
            
            item = int(item)
            if selected:
                self.selectedItems.add(item)
                self.selectedID = item
                self.onSelectionChanged.emit()
            else:
                self.selectedItems.discard(item)
            
        finally:
            self.__selectionChanging = False

        return True
        
    
    def internalEditLabelCommand(self,item,newName):
        '''
        indirectly executed by treeview inplace editor;
        return non empty string to let treeView know that rename succeeded 
        '''
        return ''
    
    def editLabelCommand(self,item,newName):
        '''
        subclasses should override internalEditLabelCommand instead
        '''
        self.__itemNameChanging = True
        try:
            return self.internalEditLabelCommand(item, newName)
        finally:
            self.__itemNameChanging = False
            

    def doSetItems(self,newItems):
        if self.__selectionChanging:
            raise Exception("cannot change items while selection is changing")
        if self.__itemNameChanging:
            # raise Exception("cannot change items while item name is changing")
            # silently exit: should be no harm ignoring new items as the only thing that changed
            # is label name, and we already reflect that in UI 
            return
        
        self.selectedID = None
        self.selectedItems.clear()
        
        #cmds.treeView(self.control,e=True,clearSelection=True)
        cmds.treeView(self.control,e=True,removeAll=True)
        for i in newItems:
            cmds.treeView(self.control,e=True,addItem=(i.id,''))
            cmds.treeView(self.control,e=True,displayLabel=(i.id,i.displayName),displayLabelSuffix=(i.id,i.suffix))
    
    
    def getSelectedID(self):
        return self.selectedID
    
    def getSelectedNames(self):
        return [i.name for i in self.items if (i.id in self.selectedItems)]
    
    def getSelectedIDs(self):
        return list(self.selectedItems)
        

    
    def selectByID(self,id):
        if self.__selectionChanging:
            return
        
        
        if self.selectedID==id:
            return
        
        self.selectedItems.add(id)
        
        
        for i in self.items:
            cmds.treeView(self.control,e=True,si=(i.id,int(i.id==id)))
        
        
class LayersTreeView(TreeViewIDList):
    def internalEditLabelCommand(self, item, newName):
        '''
        implements layer in-place rename
        '''
        # do not allow empty layer names
        if newName.strip()=='':
            return ''
        
        LayerDataModel.getInstance().setLayerName(int(item),newName)
        cmds.treeView(self.control,e=True,displayLabel=(item,newName))
        return item


    
class InfluenceFilterUi:
    VAR_PREFIX = 'ngSkinToolsInfluenceFilter_'

    def __init__(self,parent):
        self.parent = parent
        self.mainLayout = None
        self.filterChanged = Signal("Influence filter changed")
        self.isVisible = PersistentValueModel(Options.VAR_OPTION_PREFIX+"_InfluenceFilterVisible", False)
    
    def createUI(self,parent):
        result = group = self.mainLayout = cmds.frameLayout(parent=parent,label="Influence Filter", marginWidth=Constants.MARGIN_SPACING_HORIZONTAL,marginHeight=Constants.MARGIN_SPACING_VERTICAL, collapsable=True,
                                 expandCommand=self.isVisible.save,collapseCommand=self.isVisible.save,
                                 borderStyle='etchedIn')
        cmds.frameLayout(group,e=True,collapse = self.isVisible.get())
        
        column = cmds.columnLayout(parent=group,adjustableColumn=1,rowSpacing=Constants.MARGIN_SPACING_VERTICAL)
        
        form = FormLayout(parent=column)
        
        label=cmds.text(label='Influence Filter:')
        textField = self.influenceNameFilter = TextEdit(annotation="Filter influence list by name")
        clearButton = cmds.button(label='clear',width=50,command=self.clearNameFilter)
        
        
        form.attachForm(label, 10, None, 0, Constants.MARGIN_SPACING_HORIZONTAL)
        form.attachForm(clearButton, 10, Constants.MARGIN_SPACING_HORIZONTAL, 0, None)
        form.attachForm(textField,10,None,0,None)
        form.attachControl(textField,label,None,None,None,Constants.MARGIN_SPACING_HORIZONTAL)
        form.attachControl(textField,clearButton,None,Constants.MARGIN_SPACING_HORIZONTAL,None,None)
        
        
        textField.changeCommand.addHandler(self.filterChanged.emit)
        
        cmds.setParent(result)
        cmds.radioCollection()
        
        
        form = FormLayout(parent=column)
        
        self.radioAllInfluences = RadioButtonField(self.VAR_PREFIX+"allInfluences",defaultValue=1,label='Show all influences')
        self.radioAllInfluences.changeCommand.addHandler(self.radioAllInfluencesChanged)
        self.radioActiveInfluences = RadioButtonField(self.VAR_PREFIX+"activeInfluences",defaultValue=0,label='Only influences with non-zero weights')
        form.attachForm(self.radioAllInfluences, 0, 0, None, 90)
        form.attachForm(self.radioActiveInfluences, None, 0, None, 90)
        form.attachControl(self.radioActiveInfluences, self.radioAllInfluences, 0, None, None, None)
        
        return result
    
    def clearNameFilter(self,*args):
        self.influenceNameFilter.setValue('')
        self.influenceNameFilter.changeCommand.emit()
        
    def radioAllInfluencesChanged(self):
        self.parent.updateInfluenceMenuValues()
        self.filterChanged.emit()

        

    def setVisible(self,visible):
        self.isVisible.set(visible)
        cmds.frameLayout(self.mainLayout,e=True,collapse = self.isVisible.get())
    
    def toggle(self):
        self.setVisible(not self.isVisible.get())
        
    def createNameFilter(self):
        '''
        returns name filter (InflFilterMatch instance) based on control values
        '''
        filter = InfluenceNameFilter()
        filter.setFilterString(self.influenceNameFilter.getValue())
        return filter
    

class LayerListsUI:
    '''
    UI piece that defines layers&influences lists
    '''
    
    def __init__(self):
        class Controls:
            pass
        self.controls = Controls()
        self.data = LayerDataModel.getInstance()
        self.data.setLayerListsUI(self)
        
    def getMll(self):
        return self.data.mll;
        
        
    def layerSelectionChanged(self,*args):
        id = self.controls.layerDisplay.getSelectedID();
        if id is not None:
            cmds.ngSkinLayer(cl=id)
            #self.updateLayerList()
            self.updateInfluenceList()

            LayerEvents.layerSelectionChanged.emit()
            LayerEvents.currentLayerChanged.emit()
        
    def currentLayerChangedHandler(self):
        self.controls.layerDisplay.selectByID(self.data.getCurrentLayer())
        self.updateInfluenceList()
          
    def update(self):
        self.updateLayerList()
        self.updateInfluenceList()
        self.updateInfluenceMenuValues()
            
    def updateLayerList(self):
        
        if not self.data.layerDataAvailable:
            return
        
        layers = []
        try:
            layers = cmds.ngSkinLayer(q=True,listLayers=True);
            currLayer = self.data.getCurrentLayer()
        except:
            pass
            
        newItems = []
        argsPerLayer = 3
        listIndex = 1
        for i in xrange(len(layers)/argsPerLayer):
            layerID = int(layers[i*argsPerLayer])
            layerName = layers[i*argsPerLayer+1]
            indent = "  "*int(layers[i*argsPerLayer+2]) # indent, zero for base level
            enabled = "" if self.data.getLayerEnabled(layerID) else " [OFF]"
            newItems.append(IdToNameEntry(layerID,name=layerName,displayName=indent+layerName,suffix=enabled))
            
            listIndex+=1 

        
        self.controls.layerDisplay.setItems(newItems,currLayer)
        LayerEvents.layerListUpdated.emit()
        
    def updateInfluenceList(self):
        if not self.data.layerDataAvailable:
            return

        args = {}
        args['listLayerInfluences'] = True
        showAllInfluences = self.filterUi.radioAllInfluences.getValue()
        if not showAllInfluences:
            args['activeInfluences'] = True
        influences = cmds.ngSkinLayer(q=True,**args);
        
        currInfluence = LayerUtils.PAINT_TARGET_UNDEFINED if (len(influences)==0) else int(influences[0])

        # first item in influences is current influence;
        # next is a list of pair "influence name", "influence id"
        
        filter = self.filterUi.createNameFilter()
        
        newItems = []

        # append named targets
        if self.data.getCurrentLayer()>0:
            newItems.append(IdToNameEntry(LayerUtils.PAINT_TARGET_MASK, "[Layer Mask]"))
        
        names = influences[1::2]
        displayNames = InfluenceNameTransform().appendingOriginalName().transform(names)
        ids = map(int,influences[2::2])
        
        for name,displayName,influenceId in zip(names,displayNames,ids):
            if filter.isMatch(displayName):
                newItems.append(IdToNameEntry(influenceId,name=name,displayName=displayName))
                
            
            
        self.controls.influenceDisplay.setItems(newItems,currInfluence)
            
                
    def execInfluenceSelected(self,*args):
        id = self.controls.influenceDisplay.getSelectedID();
        
        
        if id is None:
            return
        
        if id==LayerUtils.PAINT_TARGET_MASK:
            cmds.ngSkinLayer(cpt="mask")
        else:
            cmds.ngSkinLayer(ci=id)
            
        #self.updateInfluenceList()
        LayerEvents.currentInfluenceChanged.emit()
        
        log.info("selected logical influence %d" % id)
        
        
    def createLayersListRMBMenu(self):
        from ngSkinTools.ui.mainwindow import MainWindow
        self.controls.layerListMenu = cmds.popupMenu( parent=self.controls.layerDisplay.control )
        
        actions = MainWindow.getInstance().actions
        actions.newLayer.newMenuItem("New Layer...")
        actions.duplicateLayer.newMenuItem("Duplicate Selected Layer(s)")
        actions.deleteLayer.newMenuItem("Delete Selected Layer(s)")
        cmds.menuItem( divider=True)
        actions.moveLayerUp.newMenuItem("Move Current Layer Up")
        actions.moveLayerDown.newMenuItem("Move Current Layer Down")
        actions.enableDisableLayer.newMenuItem("Toggle Layer On/Off")
        cmds.menuItem( divider=True)
        actions.layerProperties.newMenuItem("Properties...")

    def influenceMenuChangeCommand(self,*args):
        '''
        right mouse button menu handler for "all influences/active influences"
        '''
        all = cmds.menuItem(self.controls.menuAllInfluences,q=True,radioButton=True)
        # update filter
        self.filterUi.radioAllInfluences.setValue(all)
        self.filterUi.radioActiveInfluences.setValue(not all)
        self.updateInfluenceList()
        
    def updateInfluenceMenuValues(self):
        '''
        updates right mouse button menu values with filter values
        '''
        cmds.menuItem(self.controls.menuAllInfluences,e=True,radioButton=self.filterUi.radioAllInfluences.getValue())
        cmds.menuItem(self.controls.menuActiveInfluences,e=True,radioButton=self.filterUi.radioActiveInfluences.getValue())
        
    def createInfluenceListRMBMenu(self):
        from ngSkinTools.ui.mainwindow import MainWindow
        actions = MainWindow.getInstance().actions
        
        self.controls.inflListMenu = cmds.popupMenu( parent=self.controls.influenceDisplay.control )
        actions.copyWeights.newMenuItem('Copy Influence Weights')
        actions.cutWeights.newMenuItem('Cut Influence Weights')
        actions.pasteWeightsAdd.newMenuItem('Paste Weights (Add)')
        actions.pasteWeightsReplace.newMenuItem('Paste Weights (Replace)')
        cmds.menuItem( divider=True)
        
        
        cmds.radioMenuItemCollection()
        self.controls.menuAllInfluences = cmds.menuItem(label='List All Influences',
                enable=True, radioButton=True,command=self.influenceMenuChangeCommand )
        self.controls.menuActiveInfluences = cmds.menuItem(label='List Only Active Influences',
                enable=True, 
                radioButton=False,command=self.influenceMenuChangeCommand )
        
        cmds.menuItem( divider=True)
        
        actions.convertMaskToTransparency.newMenuItem('Convert Mask to Transparency')
        actions.convertTransparencyToMask.newMenuItem('Convert Transparency to Mask')

        cmds.menuItem( divider=True)
        
        actions.influenceFilter.newMenuItem('Show/Hide Influence Filter')
        

    def createLayerListsUI(self,parent):
        cmds.setParent(parent)
        #self.outerFrame = cmds.frameLayout(label='Skinning Layers',collapsable=False,borderVisible=True,borderStyle="etchedIn",labelAlign="center")

        if Utils.getMayaVersion()<Utils.MAYA2011:
            # pane layout is ugly if it's non-QT UI; just use simple 50:50 form layout
            paneLayout = FormLayout(numberOfDivisions=100)
        else:
            paneLayout = cmds.paneLayout(configuration="vertical2",width=100,height=200)
            
        

        leftForm = form = FormLayout()
        label = cmds.text("Layers:",align="left",font='boldLabelFont')
        list = self.controls.layerDisplay = LayersTreeView()
        list.onSelectionChanged.addHandler(self.layerSelectionChanged)
        
        form.attachForm(label,10,0,None,Constants.MARGIN_SPACING_HORIZONTAL)
        form.attachForm(list.control,None,0,0,Constants.MARGIN_SPACING_HORIZONTAL)
        form.attachControl(list.control,label,3,None,None,None)
        
        cmds.setParent("..")
        rightForm = form = FormLayout()
        label = cmds.text("Influences:",align="left",font='boldLabelFont')
        

        list = self.controls.influenceDisplay = TreeViewIDList(allowMultiSelection=True)
        list.onSelectionChanged.addHandler(self.execInfluenceSelected)
        
        self.createLayersListRMBMenu()
        self.createInfluenceListRMBMenu()

        form.attachForm(label,10,Constants.MARGIN_SPACING_HORIZONTAL,None,0)
        form.attachForm(list.control,None,Constants.MARGIN_SPACING_HORIZONTAL,0,0)
        form.attachControl(list.control,label,3,None,None,None)

        

        if Utils.getMayaVersion()<Utils.MAYA2011:
            paneLayout.attachForm(leftForm, 0, None, 0, 0)
            paneLayout.attachForm(rightForm, 0, 0, 0, None)
            cmds.formLayout(paneLayout,e=True,attachPosition=[[leftForm,'right',3,50],[rightForm,'left',3,50]])
            
        return paneLayout
    
    


    def createUI(self,parent):
        baseForm = FormLayout(parent=parent)
        self.baseLayout = baseForm
        self.controls.layerListsUI = self.createLayerListsUI(baseForm)

        
        self.filterUi = InfluenceFilterUi(self)
        self.filterUi.filterChanged.addHandler(self.updateInfluenceList)
        filterLayout = self.filterUi.createUI(baseForm)
        baseForm.attachForm(self.controls.layerListsUI, 0, 0, None, 0)
        baseForm.attachForm(filterLayout,None,0,0,0)
        baseForm.attachControl(self.controls.layerListsUI, filterLayout, None, None, Constants.MARGIN_SPACING_VERTICAL, None)

        LayerEvents.nameChanged.addHandler(self.updateLayerList,parent)
        LayerEvents.layerAvailabilityChanged.addHandler(self.update,parent)
        LayerEvents.layerListModified.addHandler(self.update,parent)
        LayerEvents.influenceListChanged.addHandler(self.updateInfluenceList,parent)
        MayaEvents.undoRedoExecuted.addHandler(self.update,parent)
        LayerEvents.currentLayerChanged.addHandler(self.currentLayerChangedHandler,parent)
        MayaEvents.nodeSelectionChanged.addHandler(self.update,parent)


        self.update()
        

    def getLayersList(self):
        return self.controls.layerDisplay
    
    def getSelectedInfluences(self):
        return self.controls.influenceDisplay.getSelectedNames()
    
    def getSelectedLayers(self):
        '''
        returns IDs for selected layers
        '''
        return self.controls.layerDisplay.getSelectedIDs()