from ngSkinTools.context import applicationContext


def skip(callable):
    def skippedFunction(*args,**kwargs):
        print "skipping %r" % callable.__name__
        
    return skippedFunction

def requiresDependency(module):
    def _requiresDependency(callable):
        def skippedFunction(*args,**kwargs):
            try:
                __import__(module)
            except:
                print "skipping %r: required dependency '%s' not found" % (callable.__name__,module)
                return
            callable(*args,**kwargs)
            
        return skippedFunction
    return _requiresDependency


def isStandalone():
    try:
        import flexmock
        return True
    except:
        return False


def standaloneOnly(callable):
    '''
    marks test skippable if we're running inside maya (3rd party libs limitations)
    '''
    
    def runIfStandalone(*args,**kwargs):
        if isStandalone():
            callable(*args,**kwargs)
            return;
        
        print "skipping %r: only available in standalone mode" % callable.__name__
        
        
    return runIfStandalone

updateServerAvailable = None

def isUpdateServerAvailable():
    global updateServerAvailable
    if updateServerAvailable is not None:
        return updateServerAvailable
    
    try:
        host = 'localhost:8080'
        path = '/version-check'
        import httplib, urllib
        conn = httplib.HTTPConnection(host)
        conn.request("GET", "/version-check")
        response = conn.getresponse()
        updateServerAvailable = True
    except:
        updateServerAvailable = False
        
    return updateServerAvailable

    


def updateCheckServiceRequired(callable):
    '''
    marks test only to be run when update check server is available
    '''
    def _updateCheckServiceRequired(self,*args,**kwargs):
        if not isUpdateServerAvailable():
            print "skipping %r: update check server not available" % callable.__name__
            return
        
        callable(self,*args,**kwargs)
        
    return _updateCheckServiceRequired
        
    
def insideMayaOnly(callable):
    '''
    marks test as to be run inside maya only
    '''
    def _insideMayaOnly(*args,**kwargs):
        try:
            from maya import cmds
            if cmds.ls('persp')!=['persp']:
                raise Exception("not in maya")
        except Exception,err:
            print "skipping %r: only available inside maya" % callable.__name__
            return
        
        callable(*args,**kwargs)
        
    return _insideMayaOnly

def raises(exception):
    def raises_decorator(callable): 
        def _assert_raises(self,*args,**kwargs):
            self.assertRaises(exception,callable,self,*args,**kwargs)
            
        return _assert_raises
    return raises_decorator
     