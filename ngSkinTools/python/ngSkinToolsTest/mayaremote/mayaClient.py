'''
prerequisites to execute on Maya side first in order for client to work:

# starting server:
from maya import cmds
cmds.commandPort(name="127.0.0.1:6005", sourceType='python',pickleOutput=True,echoOutput=False)


# stopping server:
cmds.commandPort(name="127.0.0.1:6005", cl=True)
'''

import socket
import time

class MayaClient(object):
    def __init__(self):
        self.host = '127.0.0.1'
        self.port = 6005
        
    def connectWait(self,numRetries):
        for _ in range(numRetries):
            try:
                self.connect()
                return
            except:
                print "connect failed"
                time.sleep(2)
                print "retrying"
                
        raise Exception("could not connect")
        
    def connect(self):
        self.mayaPort = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.mayaPort.connect((self.host, self.port)) 

    def disconnect(self):
        self.mayaPort.close()

    def getResponse(self):
        return self.mayaPort.recv(5000)[:-1]

    def send(self,message):
        self.mayaPort.send(message+"\n")
        
    def executeRaw(self,command):
        self.send(command)
        response = self.getResponse()
        return response
        
    def execute(self,command):
        response =  self.executeRaw(command)
        import pickle
        try:
            return pickle.loads(response)
        except Exception,err:
            print err
            raise Exception(response)
        
        
    def executeCommand(self,cmd,*args,**kwargs):
        argLine = ",".join(map(repr,args)).strip()
        
        kwargLine = ",".join(map(lambda pair:"%s=%r"%(pair[0],pair[1]),kwargs.items())).strip()
        
        allArgs = argLine
        if kwargLine!= "":
            if allArgs != "":
                allArgs += ","
            allArgs += kwargLine
            
        command = "%s(%s)" % (cmd,allArgs)
        return self.execute(command)
            
            
        
class MockMayaModule:
    def __init__(self,name):
        self.client = None
        self.moduleName = name
        
    def __getattr__(self,name):
        def mockFunction(*args,**kwargs):
            return self.client.executeCommand(self.moduleName+'.'+name,*args,**kwargs)
        return mockFunction       
            