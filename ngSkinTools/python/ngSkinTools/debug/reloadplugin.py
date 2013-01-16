import shutil
from maya import cmds
import os



def reload(source,destination):
    '''
    reloads plugin: 
    * close file
    * unload plugin
    * copy new compiled version 
    * load plugin
    * load file
    
    '''
    
    currFile = cmds.file( sceneName=True,q=True)
    
    # close file for a while
    if currFile:
        cmds.file( f=True, new=True )
     
    try:
            
        cmds.unloadPlugin(os.path.basename(destination),force=True)
        print ("plugin unloaded: %s" % destination)
        
        
                
            
        shutil.copy(source,destination)
        print ("plugin copied: %s" % source)
        cmds.loadPlugin(destination)
        print "plugin loaded: "+destination
    finally:
        if currFile:
            cmds.file(currFile, open=True)
