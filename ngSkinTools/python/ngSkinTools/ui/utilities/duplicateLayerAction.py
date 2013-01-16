from ngSkinTools.ui.actions import BaseLayerAction
from ngSkinTools.utils import Utils, MessageException
from ngSkinTools.utilities.duplicateLayers import DuplicateLayers
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.events import LayerEvents
from ngSkinTools.ui.layerListsUI import LayerListsUI

class DuplicateLayersAction(BaseLayerAction):
    
    
    @Utils.visualErrorHandling
    @Utils.undoable
    @Utils.preserveSelection
    def execute(self):
        layerListsUi = LayerDataModel.getInstance().layerListsUI
        
        setup = DuplicateLayers()
        setup.setMllInterface(LayerDataModel.getInstance().mll)
        
        layers = layerListsUi.getSelectedLayers()
        if len(layers)==0:
            raise MessageException('No layers selected')
        
        for layer in reversed(layers):
            setup.addLayer(layer)
        setup.execute()
        
        LayerDataModel.getInstance().mll.setCurrentLayer(setup.duplicateIds[-1])
        
        LayerEvents.layerListModified.emit()
        
        
        
        