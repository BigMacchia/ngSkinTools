from __future__ import with_statement
import subprocess
import os
import time
import tempfile


class MayaLauncher:
    def __init__(self):
        self.executable = r"C:\Program Files\Autodesk\Maya2011\bin\maya.exe" 
        
        self.process = None
        
        self.scriptPath = []
        self.pluginPath = []
        self.userEnvironment = ''
        
        self.env = {}
        
    
    def preparePaths(self):
        self.binDir = os.path.dirname(self.executable)
        self.baseDir = os.path.dirname(self.binDir)
        
    def makeEnvironment(self):
        env = {}
        # inherit some variables form parent process
        for var in ['ALLUSERSPROFILE','Path','SystemDrive','SystemRoot','TEMP','USERPROFILE']:
            env[var] = os.getenv(var)
            
        env['MAYA_SCRIPT_PATH'] = os.path.pathsep.join(self.scriptPath)
        env['MAYA_PLUG_IN_PATH'] = os.path.pathsep.join(self.pluginPath)
        env['Path'] += os.path.pathsep+self.binDir 
        env['MAYA_LOCATION'] = self.baseDir
        env['MAYA_APP_DIR'] = self.userEnvironment
        
        env = dict(env.items()+self.env.items())
        return env
        
    def launch(self):
        self.preparePaths()
        self.process = subprocess.Popen(args=[self.executable], cwd=self.baseDir, env=self.makeEnvironment())
        
    def close(self):
        if self.process!=None:
            self.process.kill()
            
            
class TestLauncher:
    def __init__(self):
        projectBaseDir = os.path.realpath(os.path.join(os.path.dirname(__file__),'..'))

        self.mayaLauncher = MayaLauncher()
        self.mayaLauncher.scriptPath.append(os.path.join(projectBaseDir,'python'))
        self.mayaLauncher.pluginPath.append(os.path.join(projectBaseDir,"build-target",'windows','maya2011-64bit','module','plug-ins'))
        self.mayaLauncher.userEnvironment = os.path.join(os.path.dirname(__file__),'testMayaLaunchEnvironment')

        self.testResultFile = os.path.join(tempfile.gettempdir(),'maya test result file.txt')
        self.mayaLauncher.env['MAYA_TEST_RESULT_FILE'] = self.testResultFile
        
        self.testResult = None
        self.testsSuccessful = False
        
    def pollForResultsFile(self):
        retries = 300
        print "Waiting for results file at %s..." % self.testResultFile
        while not os.path.exists(self.testResultFile):
            retries -=1
            if (retries==0):
                raise Exception("failed to receive results file")
            time.sleep(1)
            
        with open(self.testResultFile, "r") as f:
            self.testResult = f.read()
            
        print self.testResult
            
            
    def run(self):
        try:
            if os.path.exists(self.testResultFile):
                os.unlink(self.testResultFile)
                
            self.mayaLauncher.launch()
            
            self.pollForResultsFile()
            
            self.mayaLauncher.close()
            
            if not "tests failed: 0" in self.testResult:
                raise Exception,"There are failed tests"
            
        except Exception,err:
            print str(err)
            exit(1)
        
        
        
        
        

if __name__ == '__main__':
    TestLauncher().run()