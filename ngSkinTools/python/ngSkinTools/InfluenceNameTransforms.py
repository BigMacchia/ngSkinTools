'''
Created on 2013.01.01

@author: uiron
'''
from maya import cmds
from ngSkinTools.utils import MessageException


class InfluenceNameTransform(object):
    def __init__(self):
        self.appendOriginalName = False
        
        
    def appendingOriginalName(self):
        self.appendOriginalName = True
        return self
        
    
    @staticmethod
    def getShortName(name):
        try:
            return name[name.rindex("|")+1:]
        except Exception,err:
            return name
        
    def transformEach(self,name):
        
        uniqueInContext = self.getUniqueNameInContext(name) #name[len(self.commonPrefix):]
        if self.appendOriginalName:
            newName = self.getShortName(name)
            
            if newName!=uniqueInContext:
                newName += " (%s)" % uniqueInContext
            return newName
        else:
            return uniqueInContext
    
    def precalcCommonPrefix(self,names):
        def findCommonPrefix(a,b):
            for i, (x, y) in enumerate(zip(a,b)):
                if x!=y: return a[:i]
            return a
              
        commonPrefix = names[0].split("|")
        for name in names[1:]:
            commonPrefix = findCommonPrefix(commonPrefix,name.split("|"))
            
        if len(commonPrefix)==0:
            return ""
        
        return "|".join(commonPrefix)+"|"
    
    def getUniqueNameInContext(self,name):
        neighbours = self.neighbours[name]
        
        uniqueSeparator =name.rfind("|")
        while uniqueSeparator!=-1:
            uniqueName = name[uniqueSeparator+1:]
            if not neighbours[0].endswith(uniqueName) and not neighbours[1].endswith(uniqueName):
                return uniqueName
            
            uniqueSeparator =name.rfind("|",0,uniqueSeparator)
        
        return name
                
              
    
    
    def transform(self,names):
        '''
        expects a list of unique names (scene-wise) of nodes and returns them formatted as required
        '''

        def reversed(s):
            return s[::-1]
        
        orderedByEnding = map(reversed,sorted(map(reversed,names)))
        self.neighbours = {}
        for i in range(len(orderedByEnding)):
            self.neighbours[orderedByEnding[i]] = ("" if i<0 else orderedByEnding[i-1], "" if i+1>=len(orderedByEnding) else orderedByEnding[i+1])
        
        return map(self.transformEach, names)
        
      