from ngSkinTools.InfluenceNameTransforms import InfluenceNameTransform
import re
class InfluenceNameFilter:
    '''
    simple helper object to match against filter strings;
    accepts filter as a string, breaks it down into lowercase tokens, and
    matches values in non-case sensitive way
    
    e.g. filter "leg arm spines" matches "leg", "left_leg", 
    "R_arm", but does not match "spine"
    
    in a  special case of empty filter, returns true for isMatch
    '''
    
    def __init__(self):
        self.matchers = None
        
    def setFilterString(self,filter):
        
        def createPattern(expression):
            expression = "".join([char for char in expression if char.lower() in "abcdefghijklmnopqrstuvwxyz0123456789_*"])
            expression = expression.replace("*", ".*")
            return re.compile(expression,re.I)
        
        self.matchers = [createPattern(i.strip()) for i in filter.split() if i.strip()!='']
        return self
        
    def isMatch(self,value):
        if len(self.matchers)==0:
            return True
        
        value = InfluenceNameTransform.getShortName(str(value).lower())
        for pattern in self.matchers:
            if pattern.search(value) is not None:
                return True
            
        return False

