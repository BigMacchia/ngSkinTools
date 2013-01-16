from maya import cmds
from ngSkinTools.ui.layerListsUI import LayerListsUI
from ngSkinTools.ui.noLayersUI import NoLayersUI
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.events import LayerEvents

class TargetDataDisplay:
    '''
    Manages UI for either layer lists or "no layers available" UI
    '''
    
    def __init__(self):
        self.layersUI = LayerListsUI()
        self.noLayersUI = NoLayersUI()
        self.data = LayerDataModel.getInstance() 
        

    def updateUiVisibility(self):
        cmds.layout(self.noLayersUI.baseLayout,e=True,visible=self.data.layerDataAvailable==False) # 3rd option is none, hence weird compares
        cmds.layout(self.layersUI.baseLayout,e=True,visible=self.data.layerDataAvailable==True)
    
    def create(self,parent):
        LayerEvents.layerAvailabilityChanged.addHandler(self.updateUiVisibility,parent)
        result = self.uiSwitchLayout = cmds.columnLayout('layersOnOffSwitch',parent=parent,adjustableColumn=1)

        self.noLayersUI.createUI(self.uiSwitchLayout)
        self.layersUI.createUI(self.uiSwitchLayout)
        
        self.noLayersUI.update()
        self.layersUI.update()
        self.updateUiVisibility()
        
        
        return result
        
    def getSelectedInfluences(self):
        if self.data.layerDataAvailable:
            return self.layersUI.getSelectedInfluences()
        
        return []
    
