from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.utils import Utils
from maya import cmds
from ngSkinTools.ui.events import MayaEvents, restartEvents
from ngSkinTools.doclink import SkinToolsDocs
from ngSkinTools.log import LoggerFactory


log = LoggerFactory.getLogger("HeadlessDataHost")


class RefCountedHandle:
    '''
    reference counted handle to a dynamically allocated instance;
    
    creates reference after first addReference() call, and destroys it 
    when last reference is removed via removeReference()
    '''
    
    def __init__(self,instantiator):
        self.instantiator=instantiator
        self.instance = None
        self.references = set()
    
    def getCurrentInstance(self):
        '''
        returns handle to currently created instance
        '''
        return self.instance

    def addReference(self,refSource):
        if refSource in self.references:
            return
        
        if len(self.references)==0:
            self.instance = self.instantiator()
            self.instance.initialize()
            
        self.references.add(refSource)
        
        
    def removeReference(self,refSource):
        '''
        returns false, if provided reference was not found in the stack
        '''
        
        if refSource not in self.references:
            return False
        
        self.references.remove(refSource)
        if len(self.references)==0:
            self.instance.cleanup()
            self.instance=None
            
        return True
            
        
class HeadlessDataHost:

    '''
    A singleton of this object is created when at least one UI window is opened,
    and performs a cleanup once all objects are closed
    '''
    
    HANDLE = None
    
    @staticmethod
    def get():
        return HeadlessDataHost.HANDLE.getCurrentInstance() 
    
    def __init__(self):
        self.documentation = SkinToolsDocs()
        
        
    def initialize(self):
        log.debug("creating headless data host")
        self.scriptJobs = []
        
        LayerDataModel.reset()
        restartEvents()

        Utils.loadPlugin()

        self.registerScriptJob('SelectionChanged',MayaEvents.nodeSelectionChanged.emit)
        self.registerScriptJob('Undo',MayaEvents.undoRedoExecuted.emit)
        self.registerScriptJob('Redo',MayaEvents.undoRedoExecuted.emit)
        self.registerScriptJob('ToolChanged',MayaEvents.toolChanged.emit)        
        
        LayerDataModel.getInstance()
        
    def registerScriptJob(self,jobName,handler):
        job = cmds.scriptJob(e=[jobName,handler])
        self.scriptJobs.append(job)
    
    
    def cleanup(self):
        '''
        cleanup any acquired resources
        '''
        #log.debug("cleaning up data host")
        for i in self.scriptJobs:
            cmds.scriptJob(kill=i)
            
        log.debug("headless data host cleanup")


HeadlessDataHost.HANDLE = RefCountedHandle(HeadlessDataHost)        
    
