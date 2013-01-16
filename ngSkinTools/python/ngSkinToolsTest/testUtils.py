import os.path as path
import maya.cmds as cmds, maya.utils as mUtils
from ngSkinTools.utils import MessageException, Utils
from ngSkinTools.ui.basedialog import BaseDialog


def getProjectBaseDir():
    return path.dirname(path.dirname(path.dirname(__file__)))

def getBaseResourcesDir():
    return path.join(getProjectBaseDir(),'mll','test-resources')

def openMayaFile(file):
    file = path.join(getBaseResourcesDir(),file)
    cmds.file(file,open=True,f=True,ignoreVersion=True)
    
    
def runInNextModalDialog(dialogExecutable):
    if Utils.getMayaVersion()>=Utils.MAYA2011:
        mUtils.executeDeferred(lambda:dialogExecutable(BaseDialog.currentDialog))
    else:
        BaseDialog.stuffToRunInNextModalDialogHack.append(dialogExecutable)
    
def closeNextDialogWithResult(result):
    '''
    close next modal dialog with given result
    '''
    if Utils.getMayaVersion()>=Utils.MAYA2011:
        mUtils.executeDeferred(lambda:BaseDialog.currentDialog.closeDialogWithResult(result))
    else:
        Utils.displayError("hurray for maya 2009, close dialog manually with result "+result)



class AdditionalAsserts:
    def assertFloatArraysEqual(self,list1,list2):
        self.assertEquals(len(list1),len(list2))
        for index, (v1,v2) in enumerate(zip(list1,list2)):
            self.assertTrue(abs(v1-v2)<0.000001,"values not equal at %d: %r %r:" % (index,v1,v2))

    def assertArraysEqual(self,list1,list2):
        self.assertEquals(len(list1),len(list2))
        for index, (v1,v2) in enumerate(zip(list1,list2)):
            self.assertEqual(v1,v2,"values not equal at %d: %r %r:" % (index,v1,v2))
    
    def assertModelsEqual(self,model1,model2):
        '''
        validates that two layer data models are identical
        '''
        self.assertEquals(len(model1.layers),len(model2.layers))
        
        for l1,l2 in zip(model1.layers,model2.layers):
            self.assertEquals(l1.name, l2.name)
            self.assertEquals(l1.opacity, l2.opacity)
            self.assertEquals(l1.enabled, l2.enabled)
            self.assertEquals(l1.mask, l2.mask)
            
            self.assertEquals(len(l1.influences),len(l2.influences))
            for i1,i2 in zip(l1.influences,l2.influences):
                self.assertEquals(i1.logicalIndex, i2.logicalIndex)
                self.assertEquals(i1.influenceName, i2.influenceName)
                self.assertEquals(i1.weights, i2.weights)
                
    def assertRaises(self,exceptionType=None,errorMessage=None):
        class RaiseContext:
            def __init__(self,parent):
                self.parent=parent
                self.exceptionType = exceptionType
            
            def __enter__(self):
                return self    
            
            def __exit__(self,type,value,traceback):
                if self.exceptionType is not None:
                    self.parent.assertTrue(isinstance(value,self.exceptionType),"Expected exception of type %r but was %r instead" % (self.exceptionType,value))
                
                if (errorMessage!=None):
                    actualMessage = str(value)
                    if isinstance(value,MessageException):
                        actualMessage = value.message
                    if actualMessage.find(errorMessage)<0:
                        self.parent.fail("expected error message '%s', but was '%s' instead" % (errorMessage,actualMessage))
                
                return True
                 
        return RaiseContext(self)        