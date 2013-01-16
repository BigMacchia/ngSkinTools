from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.utils import MessageException
from ngSkinTools.log import LoggerFactory
from ngSkinTools.layerUtils import LayerUtils

log = LoggerFactory.getLogger("WeightsClipboard")
class WeightsClipboard:
    def __init__(self,mllInterface):
        self.copiedWeights = None
        self.mll = mllInterface
        self.layer = None
        self.influence = None
    

    def withCurrentLayerAndInfluence(self):
        self.layer = self.mll.getCurrentLayer()
        log.debug("weights clipboard setting current layer to %r" % self.layer)
        self.influence = self.mll.getCurrentPaintTarget()
        log.debug("weights clipboard setting current influence to %r" % self.influence)
        return self
    
    def getPaintTargetWeights(self,paintTarget):
        if paintTarget==LayerUtils.PAINT_TARGET_MASK:
            return self.mll.getLayerMask(self.layer)
        else:
            return self.mll.getInfluenceWeights(self.layer,paintTarget)

    def copy(self):
        self.copiedWeights = self.getPaintTargetWeights(self.influence)
        
        log.debug("copied weights: %r" % self.copiedWeights)
        if len(self.copiedWeights)==0:
            self.copiedWeights = None
            raise MessageException("Nothing copied")
    
    def cut(self):
        self.copy()
        self.mll.setInfluenceWeights(self.layer, self.influence, [0.0]*len(self.copiedWeights))
        
    
    def paste(self,replace):
        if self.copiedWeights == None:
            raise MessageException("Nothing to paste")
        
        if self.mll.getVertCount()!=len(self.copiedWeights):
            raise MessageException("Could not paste weights - vertex count does not match")
        
        newWeights =self.copiedWeights
        if not replace: 
            prevWeights = self.getPaintTargetWeights(self.influence)
            newWeights = [a+b for a,b in zip(newWeights,prevWeights)]
        
        if self.influence==LayerUtils.PAINT_TARGET_MASK:
            self.mll.setLayerMask(self.layer, newWeights)
        else:
            self.mll.setInfluenceWeights(self.layer, self.influence, newWeights)
        
