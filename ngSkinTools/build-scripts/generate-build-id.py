import hashlib
import random
import os

def generateId():
    '''
    generate build random id
    '''
    
    
    m = hashlib.sha1()
    m.update(os.urandom(1024));
    m.update(str(random.random()))
    m.update(str(random.random()))
    return m.hexdigest()



if __name__ == '__main__':
    print generateId()