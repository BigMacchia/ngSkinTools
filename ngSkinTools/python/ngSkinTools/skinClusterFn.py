from maya import cmds
from maya import OpenMayaAnim as oma
from maya import OpenMaya as om
from ngSkinTools.utils import Utils, MessageException

class SkinClusterFn(object):
    
    def __init__(self):
        self.fn = None
        self.skinCluster = None
        
    def setSkinCluster(self,skinClusterName):
        self.skinCluster = skinClusterName
        self.fn = oma.MFnSkinCluster(Utils.getMObjectForNode(skinClusterName))
        return self
        
    def getLogicalInfluenceIndex(self,influenceName):
        try:
            path = Utils.getDagPathForNode(influenceName)
        except:
            raise MessageException("Could not find influence '%s' in %s" % (influenceName, self.skinCluster))
            
        return self.fn.indexForInfluenceObject(path)
        
        
    def listInfluences(self):
        dagPaths = om.MDagPathArray()
        
        self.fn.influenceObjects(dagPaths)
        result = []
        for i in range(dagPaths.length()):
            result.append(dagPaths[i].fullPathName())
        
        return result
    
    