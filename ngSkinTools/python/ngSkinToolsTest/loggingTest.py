from ngSkinTools.log import DummyLoggerFactory,\
    SimpleLoggerFactory
import unittest
import logging

class AbstractLoggingTest:
    factory = None
    
    def testCreateLogger(self):
        l = self.factory.getLogger("testLogger")
        l.info("something's up")
        
    def testLogMethods(self):
        l = self.factory.getLogger("testLogger")
        l.info("info")
        l.debug("debug")
        l.warning("warning")
        l.error("error")
        
        
        

class DummyLoggingTest(AbstractLoggingTest,unittest.TestCase):
    factory = DummyLoggerFactory()
    def testEnabledFor(self):
        dummyLog = DummyLoggerFactory().getLogger("something")
        self.assertEquals(dummyLog.isEnabledFor(logging.DEBUG),False)
        self.assertEquals(dummyLog.isEnabledFor(logging.INFO),False)
    
class LoggingTest(AbstractLoggingTest,unittest.TestCase):
    factory = SimpleLoggerFactory()    

    def testEnabledFor(self):
        log = SimpleLoggerFactory().getLogger("something")
        self.assertEquals(log.isEnabledFor(logging.DEBUG),True)
        self.assertEquals(log.isEnabledFor(logging.INFO),True)
    