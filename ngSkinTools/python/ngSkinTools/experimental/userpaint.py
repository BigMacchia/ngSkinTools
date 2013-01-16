'''
experiments with artUserPaintCtx

some observations:
    * undo operation will not call any callback, but will record and undo any undoable command from setValue
'''
from maya import cmds,OpenMaya as om
from maya import mel
import os.path as path
import math



class PaintContext:
    MELPROCPREFIX='ngSkinTools'
    vertValues = {}
    colorValues=[]

    
    @staticmethod
    def initWeightColors():
        '''
        initializes color to weight map from image's first row. 
        image content should be pretty wide (~1000px) gradient
        '''
        PaintContext.colorValues = []
        image = om.MImage()
        image.readFromFile(path.join(path.dirname(path.dirname(__file__)),'ui','images','gradientyellow.jpg'))
        

        scriptUtil = om.MScriptUtil()
        width = om.MScriptUtil()
        height = om.MScriptUtil()
        width.createFromInt(0)
        height.createFromInt(0)
        pWidth = width.asUintPtr()
        pHeight = height.asUintPtr() 
        image.getSize( pWidth, pHeight )
        
        vWidth = width.getUint(pWidth)
        vHeight = height.getUint(pHeight)
        
        size = vWidth * vHeight
        charPixelPtr = image.pixels()
        for i in range( 0, vWidth):
                color = []
                for j in range(0,3):
                    color.append(om.MScriptUtil.getUcharArrayItem(charPixelPtr,(i*4)+j)/255.0)
                PaintContext.colorValues.append(color)
                
        PaintContext.colorValues.reverse() 
        
    
    @staticmethod
    def mapWeightToValue(value):
        """
        remap value into image color;
        old version:
        remap value [0.0 .. 1.0] into [blue...green...red] gradient
        """
        
        colors = PaintContext.colorValues
        colorIndex = int(math.floor((len(colors)-1)*value))
        return colors[colorIndex]
    
    
        if value<0.5:
            # blue-green part
            return [0,2.0*value,2.0*(.5-value)]
        
        # green-red part
        value -= 0.5
        return [2.0*value,2.0*(.5-value),0]
    
    @staticmethod
    def execute():
        PaintContext.initWeightColors()
        melfile = path.join(path.dirname(__file__),'examplePaint.mel')
        melfile = melfile.replace("\\", "/")
        mel.eval('source "%s";ngSkinTools_paintLayers();' % melfile);
        #cmds.polyColorPerVertex('pTorusShape1',r=0,g=0,b=0);
        
    @staticmethod
    def getValue(vertNum):
        '''
        this gets called by MEL procedures
        '''
        if vertNum in PaintContext.vertValues:
            return float(PaintContext.vertValues[vertNum])
        return 0.0
    
    @staticmethod
    def setValue(vertNum,value):
        '''
        this gets called by MEL procedures
        '''
        PaintContext.vertValues[vertNum] = value
        r,g,b = PaintContext.mapWeightToValue(value) 
        cmds.polyColorPerVertex('pTorusShape1.vtx[%d]'%vertNum,r=r,g=g,b=b);



def test():
    PaintContext.execute()