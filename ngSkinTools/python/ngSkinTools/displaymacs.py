'''
    MAC address display utility
    (c) 2009-2010 ngskintools.com

    this utility is meant to be run inside Autodesk Maya
    and is small graphical interface around ipconfig/ifconfig 
    to show MAC adresseses present on the system.
     
'''

import subprocess, os, re
from maya import cmds

class NotImplementedException:
    def __init__(self,message):
        self.message = message 

class DisplayMacsWindow:
    
    WINDOWNAME ='displayMacsUtility'
    TITLE = 'System MAC List'
    
    def createUI(self):
        '''
        creates UI fields and arranges layout, using current UI parent as parent
        '''
        
        form = cmds.formLayout()
        topLabel = cmds.text(label='MAC adresses found on your system:')
        self.macsOutField = cmds.scrollField(numberOfLines=5,wordWrap=False,editable=False,text="")
        closeButton = cmds.button(label='Close',command=lambda *args:self.closeWindow())
        
        margin1 = 5 
        margin2 = 10
        
        cmds.formLayout(form,e=True,attachForm=[(topLabel,'top',margin2)])
        cmds.formLayout(form,e=True,attachForm=[(topLabel,'left',margin1)])
        cmds.formLayout(form,e=True,attachNone=[(topLabel,'right')])
        cmds.formLayout(form,e=True,attachNone=[(topLabel,'bottom')])

        cmds.formLayout(form,e=True,attachNone=[(closeButton,'top')])
        cmds.formLayout(form,e=True,attachForm=[(closeButton,'left',margin1)])
        cmds.formLayout(form,e=True,attachForm=[(closeButton,'right',margin1)])
        cmds.formLayout(form,e=True,attachForm=[(closeButton,'bottom',margin1)])
        
        cmds.formLayout(form,e=True,attachControl=[(self.macsOutField,'top',margin2,topLabel)])
        cmds.formLayout(form,e=True,attachForm=[(self.macsOutField,'left',margin1)])
        cmds.formLayout(form,e=True,attachForm=[(self.macsOutField,'right',margin1)])
        cmds.formLayout(form,e=True,attachControl=[(self.macsOutField,'bottom',margin2,closeButton)])
    
    def getMacCommand(self):
        '''
        returns command that lists macs in it's output, depending on OS
        '''
        if os.name=='nt':
            return ['ipconfig','/all']

        if os.name=='posix':
            return ['ifconfig','-a']
        
        # sorry Macs, no MAC listing for you - yet.
        raise NotImplementedException('getting macs on this system is not implemented')
    
    def queryMacs(self):
        '''
        executes mac command
        '''
        process = subprocess.Popen(self.getMacCommand(),stdout=subprocess.PIPE)
        output,errors = process.communicate();
        return output
    
    def filterMacs(self,rawInput):
        '''
        filters out mac addresses from raw text input
        
        assumes that macs are listed in 0F:0F:0F:0F:0F:0F or 0F-0F-0F-0F-0F-0F format,
        delimited by whitespace
        '''
        expr = r'\s([0-9a-f]{2}([\:\-][0-9a-f]{2}){5})\s'
        result = re.findall(expr,rawInput,re.IGNORECASE)
        for i in result:
            yield i[0]
    
    def displayText(self,text):
        cmds.scrollField(self.macsOutField,e=True,text=text)
            
    def displayMacs(self):
        '''
        query mac addresses and display those in interface
        '''
        try:
            self.displayText('\n'.join(self.filterMacs(self.queryMacs())))
            
        except NotImplementedException,err:
            self.displayText('Not implemented:\n%s' % err.message)
        except Exception,err:
            self.displayText('Error:\n%s' % str(err))
            
            
    def showWindow(self):
        # delete and create ui
        self.closeWindow()
        if cmds.windowPref(self.WINDOWNAME, q=True,exists=True ):
            cmds.windowPref(self.WINDOWNAME, remove=True )
        self.window = cmds.window(self.WINDOWNAME,title=self.TITLE,width=400,height=250)
        
        self.createUI()
        self.displayMacs()
        cmds.showWindow(self.window)
        
    def closeWindow(self):
        if cmds.window(self.WINDOWNAME, exists=True):
            cmds.deleteUI(self.WINDOWNAME, window=True)
        

if __name__ == "__main__":
    DisplayMacsWindow().showWindow()
