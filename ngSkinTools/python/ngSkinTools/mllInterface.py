import maya.cmds as cmds
import maya.mel as mel
from ngSkinTools.log import LoggerFactory
from ngSkinTools.utils import MessageException

class MllInterface(object):
    '''
    
    A wrapper object to call functionality from ngSkinLayer command.
    Most operations operate on current selection, or on target mesh that
    was set in advance. All edit operations are undoable.
    
    Example usage:
    
    .. code-block:: python
    
        mll = MllInterface()
        mll.setCurrentMesh('myMesh')
        
        mll.initLayers()
        id = mll.createLayer('initial weights')
        mll.setInfluenceWeights(id,0,[0.0,0.0,1.0,1.0])
        
        ...
    
    '''
    
    log = LoggerFactory.getLogger("MllInterface")


    def __init__(self,mesh=None):
        self.setCurrentMesh(mesh)
        
    def setCurrentMesh(self,mesh):
        '''
        Set mesh we'll be working on with in this wrapper. Use None to operate on current selection instead. 
        '''
        self.mesh = mesh

    def initLayers(self):
        '''
        initializes layer data node setup for target mesh
        '''
        self.ngSkinLayerCmd(lda=True)
        
    def getLayersAvailable(self):
        '''
        returns true if layer data is available for target mesh
        '''
        try:
            return self.ngSkinLayerCmd(q=True,lda=True)
        except Exception,err:
            self.log.error(err)
            import traceback;traceback.print_exc()
            return False 
        
    def getCurrentLayer(self):
        '''
        get layer that is marked as selected on MLL side; current layer is used for many things, for example, as a paint target.
        '''
        return self.ngSkinLayerCmd(q=True,cl=True)

    def getCurrentInfluence(self):
        '''
        returns a tuple, containing current influence logical index and DAG path
        '''
        return self.ngSkinLayerCmd(q=True,ci=True)
        
        
    def getTargetInfo(self):
        '''
        Returns a tuple with mesh and skin cluster node names where skinLayer data
        is (or can be) attached.
        
        If current mesh (or selection) is not suitable for attaching layers,
        returns None
        '''
        try:
            result = self.ngSkinLayerCmd(q=True,ldt=True)
            if len(result)==2:
                return result
        except MessageException,err:
            raise err
        except:
            return None
        
        return None
        
        
    def getVertCount(self):
        '''
        For initialized layer info, returns number of vertices layer manager sees in the mesh.
        This might be different to actual vertex count in the mesh, if mesh has post-skin cluster mesh
        modifiers (as vertex merge or smooth) 
        '''
        return self.ngSkinLayerCmd(q=True,vertexCount=True)
        
    
    def getLayerName(self,layerId):
        return self.ngSkinLayerCmdMel("-id %d -q -name" % layerId)    
    
        
    def getLayerOpacity(self,layerId):
        '''
        Returns layer opacity as float between ``0.0`` and ``1.0``
        '''
        return float(self.ngSkinLayerCmdMel('-id %d -q -opacity' % layerId))

    def setLayerOpacity(self,layerId,opacity):
        '''
        Set opacity for given layer. Use values between ``0.0`` and ``1.0``
        '''
        self.ngSkinLayerCmd(e=True,id=layerId,opacity=opacity)
        
    def isLayerEnabled(self,layerId):
        '''
        Returns ``True``, if layer on/off flag is turned on
        '''
        return bool(self.ngSkinLayerCmdMel('-id %d -q -enabled' % layerId))

    def setLayerEnabled(self,layerId,enabled):
        '''
        Turn layer on/off. Use ``True`` / ``False`` for 'enabled' value.
        '''
        self.ngSkinLayerCmd(e=True,id=layerId,enabled=enabled)
    
    
    def listLayers(self):
        '''
        returns iterator to layer list; each element is a tuple: ``(layer ID, layer name)`` 
        '''
        layers = self.ngSkinLayerCmd(q=True,listLayers=True)
        argsPerLayer = 3
        for i in xrange(len(layers)/argsPerLayer):
            # layerID, layerName
            yield int(layers[i*argsPerLayer]),layers[i*argsPerLayer+1]
        
    
    def listLayerInfluences(self,layerId,activeInfluences=True):
        '''
        returns iterator to layer influences. each element is a tuple ``(influence name,influence logical index)``
        '''
        
        cmd = '-id %d -q -listLayerInfluences'
        if activeInfluences:
            cmd+= " -activeInfluences"
        influences = self.ngSkinLayerCmdMel(cmd % layerId)
        for j in xrange((len(influences)-1)/2):
            yield influences[j*2+1],int(influences[j*2+2])

    def __asFloatList(self,result):
        if result is None:
            return []
        
        return map(float,result)
    
    def __floatListAsString(self,floatList):
        def formatFloat(value):
            return str(value)
        
        return ",".join(map(formatFloat, floatList))
        

    def getLayerMask(self,layerId):
        '''
        returns layer mask weights as float list. if mask is not initialized, returns empty list
        '''
        return self.__asFloatList(self.ngSkinLayerCmdMel('-id %d -paintTarget mask -q -w' % layerId))

    def setLayerMask(self,layerId,weights):
        '''
        Set mask for given layer. Supply float list for weights, e.g. ``[0.0,1.0,0.6]``.
        Supply empty list to set mask into uninitialized state.
        '''
        self.ngSkinLayerCmd(e=True,id=int(layerId),paintTarget='mask',w=self.__floatListAsString(weights))
        
    def getInfluenceWeights(self,layerId,influenceLogicalIndex):
        '''
        returns influence weights as float list. For influence, provide logical index in skinCluster.matrix[] this influence connects to. 
        '''
        return self.__asFloatList(self.ngSkinLayerCmdMel('-id %d -paintTarget influence -iid %d -q -w ' % (layerId,influenceLogicalIndex,)))


    def setInfluenceWeights(self,layerId,influenceLogicalIndex,weights):
        '''
        Set weights for given influence in a layer. Provide weights as float list; vertex count should match result of :py:meth:`~.getVertCount`
        '''
        self.ngSkinLayerCmd(e=True,id=int(layerId),paintTarget='influence',iid=int(influenceLogicalIndex),w=self.__floatListAsString(weights))
    
    def ngSkinLayerCmd(self,**kwargs):
        args = (self.mesh,) if self.mesh is not None else ()
        
        return cmds.ngSkinLayer(*args,**kwargs)
            
    
    def ngSkinLayerCmdMel(self,melCmd):
        melCmd = "ngSkinLayer "+melCmd
        if self.mesh is not None:
            melCmd = melCmd + " " + self.mesh
        
        self.log.info(melCmd)
        return mel.eval(melCmd)
    
    
    def createLayer(self,name,forceEmpty=False):
        '''
        creates new layer with given name and returns it's ID; when forceEmpty flag is set to true, 
        layer weights will not be populated from skin cluster.
        '''
        return self.ngSkinLayerCmd(name=name,add=True,forceEmpty=forceEmpty)
    
    def deleteLayer(self,layerId):
        '''
        Deletes given layer in target mesh
        '''
        self.ngSkinLayerCmd(rm=True,id=layerId)
    
    
    def setCurrentLayer(self,layerId):
        '''
        Set current layer to given value
        '''
        return self.ngSkinLayerCmd(cl=layerId)    
    
        
    def getMirrorAxis(self):
        '''
        Get axis that is used in the mirror operation. Can be one of: 'x', 'y', 'z', or 'undefined' 
        '''
        return self.ngSkinLayerCmd(q=True,mirrorAxis=True)
    
     
    def mirrorLayerWeights(self,layerId,mirrorWidth,mirrorLayerWeights,mirrorLayerMask,mirrorDirection):        
        self.ngSkinLayerCmd(
                id = layerId,
                mirrorWidth=mirrorWidth,
                mirrorLayerWeights=mirrorLayerWeights,
                mirrorLayerMask=mirrorLayerMask,
                mirrorDirection=mirrorDirection
            )
        
    
    def beginDataUpdate(self):
        '''
        starts batch data update mode, putting layer data into suspended state - certain 
        internal updates are switched off, making multiple layer data changes like setLayerWeights 
        or setLayerOpacity run faster; updates will take place when endDataUpdate is called.
        
        begin..endDataUpdate() pairs can be stacked (e.g. methods inside begin..end can call begin..end
        themselves) - updates will resume only when most outer pair finishes executing.
        '''
        self.ngSkinLayerCmd(beginDataUpdate=True)

    def endDataUpdate(self):
        '''
        end batch update.
        '''
        self.ngSkinLayerCmd(endDataUpdate=True)
        
    def batchUpdateContext(self):
        '''
        a helper method to use in a "with" statement, e.g.:

        .. code-block:: python
            
            with mll.batchUpdateContext():
                mll.setLayerWeights(...)
                mll.setLayerOpacity(...)
                
        this is the same as:
        
        .. code-block:: python
        
            mll.beginDataUpdate()
            try:
                mll.setLayerWeights(...)
                mll.setLayerOpacity(...)
            finally:
                mll.endDataUpdate()
        '''
        
        return BatchUpdateContext(self)
        
class BatchUpdateContext:
    '''
    A helper class for MllInterface.batchUpdateContext() method.
    '''
    def __init__(self,mll):
        self.mll = mll
        
    def __enter__(self):
        self.mll.beginDataUpdate()
        return self.mll
    
    def __exit__(self, type, value, traceback):
        self.mll.endDataUpdate()
        
