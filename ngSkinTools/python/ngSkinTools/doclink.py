

from ngSkinTools.utils import Utils
from os import path
import webbrowser

class DocLink:
    def __init__(self,title, id, anchor=None):
        self.id = id
        self.title = title
        self.anchor = None
        
    def getId(self):
        return self.id
    
    def getAnchor(self):
        return self.anchor
    
    def getTitle(self):
        return self.title
    

class DocumentationBase:
    def __init__(self):
        self.documentationRoot = None
    
    def setDocumentationRoot(self,docroot):
        '''
        sets documentation root. valid entries are: 
            * format ready documentation string, containing exactly one string argument: 
                http://www.something.com/?opendoc=%s
                c:\docs\%s.html
            * existing folder on local machine
                c:\docs
        '''

        self.documentationRoot = docroot
        
    @staticmethod
    def isLocal(link):
        '''
        primitive check if documentation is local or remote via http
        '''
        return not link.lower().startswith('http://')
    
    def getURL(self,link):
        '''
        transform documentation link (which is just a document name, like "index")
        into an actual link to documentation resource, e.g.:
        'index' -> 'http://www.documentationhost.com/documentation/get.php?index'
        'about' -> 'file:///c:/documentation/about.html'
        '''

        result = self.documentationRoot
        if result.find("%s")==-1:
            result = path.join(result,'%s.html')
        
        
        result = result % link.id
        if link.anchor:
            result += '#'+link.anchor
        return result
    
    def linkExists(self,link):
        if not self.isLocal(link):
            return True
        
        return path.exists(link)
    
    def openLink(self,link):
        '''
        open given documentation link (should be an instance of DocLink)
        if documentation root is local, checks for link validity
        on success, opens a web browser 
        '''
        if self.documentationRoot is None:
            return
        
        uri = self.getURL(link)
        
        # if possible, check for link availability
        if not self.linkExists(uri):
            Utils.displayError('documentation path does not exist: '+uri)
            return
        
        if self.isLocal(uri):
            uri = uri.replace("\\", "/");
            if not uri.lower().startswith("file:///"):
                uri = "file:///"+uri
            
        webbrowser.open_new(uri)
        
        
        
class SkinToolsDocs(DocumentationBase):
    WEIGHTSRELAX_INTERFACE = DocLink('skin weights relax','weightsrelax/interface')
    ASSIGNWEIGHTS_CLOSESTJOINT_INTERFACE = DocLink('assign weights by closest joint','aw-interface','closestJoint')
    ASSIGNWEIGHTS_MAKERIGID_INTERFACE = DocLink('unify weights','aw-interface','unifyWeights')
    ASSIGNWEIGHTS_LIMITWEIGHTS_INTERFACE = DocLink('assign weights: limit weights','aw-interface','limitWeights')
    MIRRORWEIGHTS_INTERFACE = DocLink('mirror weights','mirroring','limitWeights')

    INITWEIGHTTRANSFER_INTERFACE = DocLink('Init Weight Transfer','iwt-interface','')

    CURRENTSKINSETTINGS_INTERFACE = DocLink('Current skin settings','settings/skin','')
    
    DOCUMENTATION_ROOT = DocLink("Current documentation",'index')
    
    def __init__(self):
        DocumentationBase.__init__(self)
        
        self.setDocumentationRoot("http://www.ngskintools.com/documentation/current/")
        
        #if Utils.DEBUG_MODE:
        #    docroot = path.dirname(path.dirname(path.dirname(__file__)))
        #    docroot = path.join(docroot,'documentation','user-doc','build','html')
        #    self.setDocumentationRoot(docroot) 
