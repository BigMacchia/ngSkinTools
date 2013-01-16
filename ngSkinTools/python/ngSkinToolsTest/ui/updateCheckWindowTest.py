import unittest
from ngSkinToolsTest.decorators import standaloneOnly
try:
    from flexmock import flexmock
except:
    pass

from ngSkinTools.ui.updateCheckWindow import UpdateCheckThread
from ngSkinTools.context import applicationContext
from ngSkinToolsTest.debugApplicationSetup import DebugApplicationSetup

class UpdateCheckThreadTest(unittest.TestCase):
    def setUp(self):
        unittest.TestCase.setUp(self)
        applicationContext.setup = DebugApplicationSetup()
    
    
    def createThread(self,silent):
        unittest.TestCase.setUp(self)
        
        self.thread = UpdateCheckThread(silent)
        self.thread.updateResultsDisplay = flexmock()
        
        return self.thread 
        
        
    
    @standaloneOnly
    def testRun(self):
        '''
        test success
        '''
        self.createThread(False)
        
        
        checker = flexmock()
        checker.should_receive('execute').once()
        checker.should_receive('isUpdateAvailable').and_return(True)
        self.thread.checker = checker
        
        
        self.thread.updateResultsDisplay.should_receive('showInfoWindow').with_args('Please wait...','Information is being retrieved from ngSkinTools server.')
        self.thread.updateResultsDisplay.should_receive('showResultsWindow').with_args(checker)
        
        self.thread.run()
        
    @standaloneOnly
    def testRunWithError(self):
        '''
        test error display
        '''
        self.createThread(False)
        
        
        checker = flexmock()
        checker.should_receive('execute').and_raise(Exception("something failed"))
        self.thread.checker = checker
        
        
        self.thread.updateResultsDisplay.should_receive('showInfoWindow').with_args('Please wait...','Information is being retrieved from ngSkinTools server.')
        self.thread.updateResultsDisplay.should_receive('showInfoWindow').with_args("Error occurred", "Error occurred while getting information from the server:something failed")
        
        self.thread.run()
        

    @standaloneOnly
    def testRunSilent(self):
        '''
        run silent execution, even if update check fails
        '''
        
        self.createThread(True)
        checker = flexmock()
        checker.should_receive('execute').and_raise(Exception("something happened"))
        self.thread.checker = checker
        self.thread.updateResultsDisplay.should_receive('showInfoWindow').never()
        
        self.thread.run()

        