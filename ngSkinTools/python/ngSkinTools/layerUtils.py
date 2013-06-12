from maya import cmds,OpenMaya as om
from ngSkinTools.log import LoggerFactory


log = LoggerFactory.getLogger("LayerUtils")

class LayerUtils:
     
    PAINT_TARGET_UNDEFINED = -1
    PAINT_TARGET_MASK = -2
    
    @staticmethod
    def iterCustomNodes():
        for nodeType in ['ngSkinLayerData','ngSkinLayerDisplay']:
            items = cmds.ls(type=nodeType)
            if items is not None:
                for i in items:
                    yield i
    
    @staticmethod
    def deleteCustomNodes():
        log.info("removing ngSkinTools nodes from current scene")
                
        nodes = list(LayerUtils.iterCustomNodes())
        if len(nodes)>0:
            cmds.delete(nodes)
        
    @staticmethod
    def hasCustomNodes():
        for _ in LayerUtils.iterCustomNodes():
            return True
        
        return False
        

