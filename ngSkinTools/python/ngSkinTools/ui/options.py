import maya.cmds as cmds
from ngSkinTools.utils import MessageException, Utils


class ValueModel(object):
    def __init__(self,value=None):
        self.value = value

    def get(self):
        return self.value

    def set(self,value):
        self.value = value
        
    def getInt(self):
        try:
            return int(self.get())
        except:
            return 0

class PersistentValueModel(ValueModel):
    '''
    persistent value can store itself into Maya's "option vars" array
    '''
    def __init__(self,name,defaultValue=None):
        ValueModel.__init__(self)
        self.name = name
        self.defaultValue=defaultValue
        self.load()
        
    def save(self):
        Options.saveOption(self.name, self.value)
        
    def load(self):
        self.value = Options.loadOption(self.name, self.defaultValue)
        
    def set(self,value):
        ValueModel.set(self, value)
        self.save()
        


class Options:
    '''
    Contains utility methods to load/save maya's optionVar's more easily
    '''

    VAR_OPTION_PREFIX = 'ngSkinToolsOption_'
    
    OPTION_CHECKFORUPDATES = None
    
    OPTION_USETREEVIEW_LAYERS = None

    @staticmethod
    def loadOption(varName,defaultValue):
        '''
        loads value from optionVar
        '''
        
        if cmds.optionVar(exists=varName):
            return cmds.optionVar(q=varName)
        
        return defaultValue; 
    
    @staticmethod
    def saveOption(varName,value):
        '''
        saves option via optionVar
        '''

        # variable does not exist, attempt to save it
        key=None
        if isinstance(value,float):
            key='fv'
        elif isinstance(value,int):
            key='iv'
        elif isinstance(value,basestring):
            key='sv'
        else:
            raise MessageException("could not save option %s: invalid value %r" % (varName,value))
        
        cmds.optionVar(**{key:(varName,value)})


        
        
Options.OPTION_CHECKFORUPDATES = PersistentValueModel(Options.VAR_OPTION_PREFIX+'checkForUpdates',defaultValue=1)
Options.OPTION_USETREEVIEW_LAYERS = PersistentValueModel(Options.VAR_OPTION_PREFIX+'useTreeviewLayers',defaultValue=1)
