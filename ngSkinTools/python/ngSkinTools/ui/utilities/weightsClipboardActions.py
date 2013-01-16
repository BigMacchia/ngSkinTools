from ngSkinTools.ui.actions import BaseLayerAction
from ngSkinTools.utils import Utils, MessageException
from ngSkinTools.utilities.duplicateLayers import DuplicateLayers
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.events import LayerEvents
from ngSkinTools.ui.layerListsUI import LayerListsUI

class CopyWeights(BaseLayerAction):
    @Utils.visualErrorHandling
    @Utils.undoable
    @Utils.preserveSelection
    def execute(self):
        LayerDataModel.getInstance().clipboard.withCurrentLayerAndInfluence().copy()


class CutWeights(BaseLayerAction):
    
    @Utils.visualErrorHandling
    @Utils.undoable
    @Utils.preserveSelection
    def execute(self):
        LayerDataModel.getInstance().clipboard.withCurrentLayerAndInfluence().cut()

class PasteWeightsAdd(BaseLayerAction):
    
    @Utils.visualErrorHandling
    @Utils.undoable
    @Utils.preserveSelection
    def execute(self):
        LayerDataModel.getInstance().clipboard.withCurrentLayerAndInfluence().paste(replace=False)

class PasteWeightsReplace(BaseLayerAction):
    
    @Utils.visualErrorHandling
    @Utils.undoable
    @Utils.preserveSelection
    def execute(self):
        LayerDataModel.getInstance().clipboard.withCurrentLayerAndInfluence().paste(replace=True)
