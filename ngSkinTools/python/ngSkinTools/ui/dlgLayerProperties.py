from maya import cmds
from ngSkinTools.ui.basedialog import BaseDialog
from ngSkinTools.ui.basetab import BaseTab
from ngSkinTools.ui.constants import Constants
from ngSkinTools.ui.events import Signal
from ngSkinTools.ui.options import ValueModel
from ngSkinTools.ui.uiCompounds import FloatSliderField
from ngSkinTools.ui.uiWrappers import StoredTextEdit



        
class LayerPropertiesDialog(BaseDialog):
    def __init__(self,newLayerMode):
        BaseDialog.__init__(self)
        self.title = "New Layer" if newLayerMode else "Layer Properties"
        self.buttons = [self.BUTTON_OK,self.BUTTON_CANCEL]
        self.layerNameValue = ValueModel()
        self.layerOpacityValue = ValueModel()
        self.newLayerMode = newLayerMode
        self.onOpacityChange = Signal("Layer Opacity changed")
        
        
    def createInnerUi(self,parent):
        rows=cmds.columnLayout(parent=parent,
            adjustableColumn=1,rowSpacing=Constants.MARGIN_SPACING_VERTICAL,
            width=400)
        
        BaseTab.createTitledRow(parent=rows, title="Layer Name")
        self.controls.name = StoredTextEdit(self.layerNameValue)
        self.controls.name.editUI(enterCommand=lambda *args:self.closeDialogWithResult(self.BUTTON_OK),alwaysInvokeEnterCommandOnReturn=True)
        
        if not self.newLayerMode:
            self.controls.opacity = FloatSliderField(range=[0,1])
            self.controls.opacity.model = self.layerOpacityValue
            self.controls.opacity.onChange.addHandler(self.onOpacityChange.emit,ownerUI=self.controls.opacity.floatField)
            BaseTab.createTitledRow(parent=rows, title="Opacity",innerContentConstructor=self.controls.opacity.create)
            
        self.controls.name.focus()
        return rows
    
    
    def closeDialogWithResult(self, buttonID):
        if buttonID==self.BUTTON_OK:
            self.controls.name.setValue(self.controls.name.getValue().strip())
            
            
            # check if valid layer name was entered
            if self.layerNameValue.get().strip()=='':
                Utils.confirmDialog( title='Validation Error', message="layer name cannot be blank", button=['Ok'], defaultButton='Ok')
                return
        
        
        BaseDialog.closeDialogWithResult(self, buttonID)
        
