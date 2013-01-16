from maya import cmds,OpenMaya as om
from ngSkinTools.log import LoggerFactory


log = LoggerFactory.getLogger("LayerUtils")

class LayerUtils:
     
    PAINT_TARGET_UNDEFINED = -1
    PAINT_TARGET_MASK = -2
    
    @staticmethod
    def deleteCustomNodes():
        def deleteNodes(nodes):
            if nodes is not None and len(nodes)>0:
                cmds.delete(nodes)
                
        log.info("removing ngSkinTools nodes from current scene")
                
        deleteNodes(cmds.ls(type='ngSkinLayerData'))
        deleteNodes(cmds.ls(type='ngSkinLayerDisplay'))
        

