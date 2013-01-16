'''
    information about toolkit version
'''
from ngSkinTools.ui.options import Options, PersistentValueModel

class Version:
    RELEASE_NAME = "ngSkinTools %(version)s"
    COPYRIGHT = "Copyright \xA9 2009-2013 Viktoras Makauskas"
    PRODUCT_URL = "http://www.ngskintools.com"
    
    
    @staticmethod
    def getReleaseName():
        '''
        returns release name with version information
        '''
        return Version.RELEASE_NAME % {'version':Version.pluginVersion()}

    @staticmethod
    def pluginVersion():
        '''
        Unique version of plugin, e.g. "1.0beta.680". Also represents 
        required version of mll plugin. Automatically set at build time
        '''
        
        # automatically updated value: do not edit 
        pluginVersion_doNotEdit = "1.0beta.680"
        return pluginVersion_doNotEdit;
    
    @staticmethod
    def buildWatermark():
        '''
        returns a unique ID of this build. 
        will be set by a build system.
        '''
        
        buildWatermark_doNotEdit = "db98582f0fc50f12560dc422d666e6e8008f9235"
        return buildWatermark_doNotEdit;
    
    @staticmethod
    def uniqueClientId():
        '''
        returns a unique ID for this installation. randomly generated at first run.
        '''
        model = PersistentValueModel(Options.VAR_OPTION_PREFIX+"updateCheckUniqueClientId", None)
        if model.get()==None:
            model.set(generateUniqueClientId())
        return model.get()
    

# returns random hexadecimal 40-long string    
def generateUniqueClientId():
    import random
    result = ""
    for i in range(10):
        result += "%0.4x" % random.randrange(0xFFFF)
        
    return result
    