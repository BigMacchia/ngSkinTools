from ngSkinTools.versioncheck import VersionChecker, HttpPostTransport
from ngSkinTools.version import Version
from ngSkinTools.utils import Utils



class ApplicationSetup:
    def __init__(self):
        self.updateCheckHost = 'ngskintools.com'
        self.updateCheckPath = '/ngskintools-version-check-1'
        
        
class ApplicationContext:
    def __init__(self):
        self.setup = ApplicationSetup()
    
    def createVersionChecker(self):
        checker = VersionChecker()
        checker.currentId = Version.buildWatermark()
        checker.uniqueClientId = Version.uniqueClientId() 
        
        checker.transport = HttpPostTransport()
        checker.transport.host = self.setup.updateCheckHost
        checker.transport.path = self.setup.updateCheckPath
        
        return checker



applicationContext = ApplicationContext()