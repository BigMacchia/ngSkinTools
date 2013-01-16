from ngSkinTools.utils import Utils
from maya import cmds


class Signal:
    '''
    Signal class collects observers, interested in some particular event,and handles
    signaling them all when some event occurs. Both handling and signaling happens outside
    of signal's own code
    '''
    
    TOTAL_HANDLERS = 0
    
    def __init__(self,name=None):
        self.handlers = []
        self.name = name
        self.executing = False
        
    
    def emitDeffered(self,*args):
        import maya.utils as mu
        mu.executeDeferred(self.emit,*args)
        
        
    def emit(self,*args):
        if self.executing:
            raise Exception,'Nested emit on %s detected' % self.name
        
        self.executing = True
        try:
            for i in self.handlers:
                i(*args)
        finally:
            self.executing = False
            

    class UiBoundHandler:
        '''
        Proxy wrapper for event handlers that has a method to deactivate 
        itself after when associated UI is deleted
        '''
        def __init__(self,handler,ownerUI,deactivateHandler):
            cmds.scriptJob(uiDeleted=[ownerUI,self.deactivate])
            self.handler=handler
            self.deactivateHandler=deactivateHandler
        
        def deactivate(self):
            self.deactivateHandler(self)
            
            
        def __call__(self):
            self.handler()
            
            
    def addHandler(self,handler,ownerUI=None):
        # if there's owning UI, wrap everything into self-deactivating handler
        if (ownerUI!=None):
            handler=self.UiBoundHandler(handler,ownerUI,self.removeHandler)
            
        self.handlers.append(handler)

    def removeHandler(self,handler):
        self.handlers.remove(handler)
        


class LayerEventsHost:
    """
    layer system related events
    """
    
    def restart(self):
        self.nameChanged = Signal('layerNameChanged')
        self.layerListModified = Signal('layerDataModified')
        self.currentLayerChanged = Signal('currentLayerChanged')
        self.currentInfluenceChanged = Signal('currentInfluenceChanged')
        self.layerSelectionChanged = Signal('layerSelectionChanged')
        self.layerListUpdated = Signal('layerListUpdated')
        self.layerAvailabilityChanged = Signal('layerAvailabilityChanged')
        self.influenceListChanged = Signal('influenceListChanged')
        self.mirrorCacheStatusChanged = Signal('mirrorCacheStatusChanged')


class MayaEventsHost:
    '''
    global maya-specific events
    '''
    
    def restart(self):
        # emits when node selection changes, while main window is open
        self.nodeSelectionChanged = Signal('nodeSelectionChanged')
        # emits when undo or redo is executed
        self.undoRedoExecuted = Signal('undoRedoExecuted')
        self.toolChanged = Signal('toolChanged')


def restartEvents():
    '''
    (re)creates signal holders in LayerEvents and MayaEvents  
    '''
    MayaEvents.restart()
    LayerEvents.restart()


MayaEvents = MayaEventsHost()
LayerEvents = LayerEventsHost() 
