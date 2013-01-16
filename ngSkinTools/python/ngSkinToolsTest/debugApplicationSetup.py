'''
Created on 2012.10.11

@author: uiron
'''
from ngSkinTools.context import ApplicationSetup


class DebugApplicationSetup(ApplicationSetup):
    def __init__(self):
        #self.updateCheckHost = 'localhost:8080'
        #self.updateCheckPath = '/version-check'
        self.updateCheckHost = 'ngskintools.com'
        self.updateCheckPath = '/ngskintools-version-check-1'
        