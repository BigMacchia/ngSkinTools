from __future__ import with_statement

def mergeFiles(inputFileList,output):
    import os
    outDir = os.path.dirname(output)
    if not os.path.exists(outDir):
        os.makedirs(outDir)

    contents = []
    interpreterLine = None
    for input in inputFileList:
            with open(input,'r') as inputFile:
                lines = inputFile.readlines()
                for line in lines:
                    if line.startswith("#!/"):
                        interpreterLine = line
                        print ("interpreter line "+interpreterLine+" on "+input)
                    else:
                        contents.append(line)

    with open(output,'w') as outFile:
        if interpreterLine is not None:
            outFile.write(interpreterLine)
            outFile.write(os.linesep)
        
        outFile.writelines(contents)

        
# accept two or more files from command line        
if __name__=="__main__":
    import sys
    if len(sys.argv)==3:
        mergeFiles(sys.argv[1:], sys.argv[-1])    
    if len(sys.argv)>=4:
        mergeFiles(sys.argv[1:-1], sys.argv[-1])