from __future__ import with_statement
import re
import sys
import getopt

def updateVersion(pluginVersion,mayaVersion,mayaPlatform,buildWatermark,destFile):
    
    try:
        with open(destFile) as f:
            fileContents = f.read()
    except Exception,e:
        print e
        print "error updating version values in %s: file read failed" % destFile
        return
        

    fileContents = re.sub(re.compile('(INSTALL_MAYA_VERSION\s*=)\s*\'[0-9]+\''),
                          '\\1 \'%s\'' % mayaVersion,
                          fileContents)
    
    fileContents = re.sub(re.compile('(INSTALL_MAYA_PLATFORM\s*=)\s*[0-9]+'),
                          '\\1 '+mayaPlatform,
                          fileContents)
    
    fileContents = re.sub(re.compile('((pluginVersion_doNotEdit|INSTALL_PLUGIN_VERSION)\s*=\s*)"[^"]*"'),
                          '\\1"'+pluginVersion+'"',
                          fileContents)
    
    fileContents = re.sub(re.compile('((buildWatermark_doNotEdit)\s*=\s*)"[^"]*"'),
                          '\\1"'+buildWatermark+'"',
                          fileContents)
    

    try:
        with open(destFile,"w") as f:
            f.write(fileContents)
                
    except Exception,e:
        print e
        print "error updating version values in %s: file write failed" % destFile
        return
    
    print ("%s updated with version: %s"% (destFile,pluginVersion))

if __name__ == '__main__':
    if len(sys.argv)<5:
        raise Exception("not enough arguments")
    
    updateVersion(*sys.argv[1:])