from maya import cmds
from ngSkinTools.utils import Utils, MessageException
from ngSkinTools.ui.basetab import BaseTab
from ngSkinTools.doclink import SkinToolsDocs
from ngSkinTools.ui.intensityslider import IntensitySlider
from ngSkinTools.ui.uiWrappers import IntField, CheckBoxField, \
    RadioButtonField, FormLayout, Layout
from ngSkinTools.ui.constants import Constants
from ngSkinTools.ui.softSelectionRow import SoftSelectionRow
from ngSkinTools.importExport import LayerData
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.events import LayerEvents, MayaEvents

     

class TabSettings(BaseTab):
    '''
    '''
    
    # prefix for preset variables for this tab
    SETTINGS_PREFIX = 'ngSkinToolsSettingsTab_'
    
    def __init__(self):
        BaseTab.__init__(self)
        
    def updateUIEnabled(self):
        layersAvailable = LayerDataModel.getInstance().getLayersAvailable()
        
        Layout.setEnabled(self.controls.selectedSkinSettingsGroup,layersAvailable)
        if not layersAvailable:
            return
        
        self.controls.influenceLimitRow.setEnabled(self.controls.useInfluenceLimit.getModelValue())
        
    def applyCurrentSkinSettings(self):
        print "setting limit enabled to",self.controls.useInfluenceLimit.getModelValue()
        print "setting limit number to",self.controls.numMaxInfluences.getModelValue()
        
    def refreshSettingsFromSelection(self):
        layersAvailable = LayerDataModel.getInstance().getLayersAvailable()
        currentLimit = 0 if not layersAvailable else LayerDataModel.getInstance().mll.getInfluenceLimitPerVertex()
        
        self.controls.numMaxInfluences.setValue(max(1,currentLimit))
        self.controls.useInfluenceLimit.setValue(currentLimit!=0)
        self.updateUIEnabled()

    def createUI(self, parent):
        self.setTitle('Settings')
        self.outerLayout = FormLayout()
        scrollLayout = BaseTab.createScrollLayout(self.outerLayout)
        self.baseLayout = cmds.columnLayout(adjustableColumn=1)
        self.outerLayout.attachForm(scrollLayout, 0, 0, 0, 0)
        
        
        self.controls.selectedSkinSettingsGroup = group = self.createUIGroup(self.baseLayout, 'Selected Skin Settings')
        self.controls.useInfluenceLimit = CheckBoxField(None,defaultValue=0,label="Use maximum influences per vertex limit",
                annotation='Turn this on to enforce a max influences per vertex limit')
        self.controls.useInfluenceLimit.changeCommand.addHandler(self.updateUIEnabled, ownerUI=parent)
        
        
        self.controls.influenceLimitRow = self.createFixedTitledRow(group, 'Influence limit')
        self.controls.numMaxInfluences = IntField(None,minValue=1,maxValue=None,annotation="Number of max influences per vertex")
        
        cmds.setParent(group)
        cmds.rowLayout(nc=2,adjustableColumn=2,columnWidth2=[Constants.BUTTON_WIDTH_SMALL,50], columnAttach2=["both","both"],columnAlign2=["center","center"])
        BaseTab.createHelpButton(SkinToolsDocs.CURRENTSKINSETTINGS_INTERFACE)
        cmds.button(height=Constants.BUTTON_HEIGHT,label='Apply',command=lambda *args:self.applyCurrentSkinSettings())        
        
        MayaEvents.nodeSelectionChanged.addHandler(self.refreshSettingsFromSelection,parent)
        MayaEvents.undoRedoExecuted.addHandler(self.refreshSettingsFromSelection,parent)
        
        self.refreshSettingsFromSelection()
        
        return self.outerLayout
