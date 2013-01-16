from ngSkinTools.mllInterface import MllInterface
from ngSkinTools.utils import MessageException
class WeightsClipboard:
    def __init__(self,mllInterface):
        self.copiedWeights = None
        self.mll = mllInterface
        self.layer = None
        self.influence = None
    

    def withCurrentLayerAndInfluence(self):
        self.layer = self.mll.getCurrentLayer()
        self.influence = self.mll.getCurrentInfluence()[0]
        return self

    def copy(self):
        self.copiedWeights = self.mll.getInfluenceWeights(self.layer,self.influence)
    
    def cut(self):
        self.copy()
        self.mll.setInfluenceWeights(self.layer, self.influence, [0.0]*len(self.copiedWeights))
        
    
    def paste(self,replace):
        if self.mll.getVertCount()!=len(self.copiedWeights):
            raise MessageException("Could not paste weights - vertex count does not match")
        
        newWeights =self.copiedWeights
        if not replace: 
            prevWeights = self.mll.getInfluenceWeights(self.layer, self.influence)
            newWeights = [a+b for a,b in zip(newWeights,prevWeights)]
        
        self.mll.setInfluenceWeights(self.layer, self.influence, newWeights)
        
