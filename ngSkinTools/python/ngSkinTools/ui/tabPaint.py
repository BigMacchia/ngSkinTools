from maya import cmds
from ngSkinTools.ui.basetab import BaseTab
from ngSkinTools.utils import Utils
from ngSkinTools.ui.events import LayerEvents, MayaEvents
from ngSkinTools.ui.layerDataModel import LayerDataModel
from ngSkinTools.ui.uiWrappers import RadioButtonField
from ngSkinTools.doclink import SkinToolsDocs
from ngSkinTools.ui.options import PersistentValueModel
from ngSkinTools.ui.uiCompounds import FloatSliderField
from ngSkinTools.ui.SelectHelper import SelectHelper

class TabPaint(BaseTab):
    TOOL_PAINT = 'ngSkinToolsLayerPaintCtx'

    VAR_PREFIX = 'ngSkinToolsPaintTab_'
    
    
    PAINTMODE_REPLACE = 1
    PAINTMODE_ADD = 2
    PAINTMODE_SCALE = 3
    PAINTMODE_SMOOTH = 4
    
    
    def __init__(self):
        BaseTab.__init__(self)
        
        self.controls.brushShapeButtons = []
        
        self.intensityReplace = PersistentValueModel(self.VAR_PREFIX+'intensityReplace',defaultValue=1.0)
        self.intensityAdd = PersistentValueModel(self.VAR_PREFIX+'intensityAdd',defaultValue=.1)
        self.intensityScale = PersistentValueModel(self.VAR_PREFIX+'intensityScale',defaultValue=.95)
        self.intensitySmooth = PersistentValueModel(self.VAR_PREFIX+'intensitySmooth',defaultValue=.5)
        
        self.brushShape = PersistentValueModel(self.VAR_PREFIX+'brushShape',defaultValue=1)
        

        
    def updateHighlight(self,includeInfluence=True):
        '''
        updates highlight - if painting, adds  current influence to highlight
        '''
        if cmds.currentCtx()!=self.TOOL_PAINT:
            return

        if not LayerDataModel.getInstance().layerDataAvailable:
            return

        newHighlightItems = []
        if includeInfluence:
            currentInfluence = cmds.ngSkinLayer(q=True,ci=True)
            # returns influence path, if current paint target has any
            if (len(currentInfluence)>1):
                newHighlightItems.append(currentInfluence[1])
                
            
        SelectHelper.replaceHighlight(newHighlightItems)
        
    
    def paintCtxSetupProcedure(self,toolContext):
        cmds.ngSkinLayer(colorDisplayNode=1)
        cmds.ngSkinLayer(displayUpdate=True,paintingMode=1)
        self.updateHighlight()

    def paintCtxCleanupProcedure(self,toolContext):
        cmds.ngSkinLayer(colorDisplayNode=0)
        cmds.ngSkinLayer(displayUpdate=True,paintingMode=0)
        self.updateHighlight(False)
        
    
    def doStartPaint(self):
        if not cmds.artUserPaintCtx(self.TOOL_PAINT,exists=True):
            cmds.artUserPaintCtx(self.TOOL_PAINT);

        cmds.artUserPaintCtx(self.TOOL_PAINT,e=True,
            tsc=Utils.createMelProcedure(self.paintCtxSetupProcedure, [('string','toolContext')]),
            toolCleanupCmd=Utils.createMelProcedure(self.paintCtxCleanupProcedure, [('string','toolContext')]),
            initializeCmd="ngLayerPaintCtxInitialize",
            finalizeCmd="ngLayerPaintCtxFinalize",
            setValueCommand="ngLayerPaintCtxSetValue", 
            getValueCommand="ngLayerPaintCtxGetValue",
            whichTool="userPaint",
            fullpaths = True
            )
        self.configurePaintValues()

        cmds.setToolTo(self.TOOL_PAINT);

    def doFlood(self):
        self.configurePaintValues()
        cmds.ngSkinLayer(paintFlood=True)
        
    def execPaint(self,*args):
        if self.isPainting():
            self.doFlood()
        else:
            self.doStartPaint()
        
        
        
    def isPainting(self):
        return cmds.currentCtx()==self.TOOL_PAINT
        
    def updateToTool(self):
        '''
        update controls to current tool
        '''
        isPainting = self.isPainting()
        cmds.control(self.cmdLayout.innerLayout,e=True,enable=isPainting)
        cmds.button(self.cmdLayout.buttons[1],e=True,label="Flood" if isPainting else "Paint")

        if (isPainting):        
            self.controls.brushRadiusSlider.setValue(cmds.artUserPaintCtx(self.TOOL_PAINT,q=True,radius=True))
        self.controls.brushRadiusSlider.setEnabled(isPainting)
        
        layersAvailable = LayerDataModel.getInstance().layerDataAvailable
        cmds.layout(self.cmdLayout.buttonForm,e=True,enable=layersAvailable)
        
       
    def getPaintModeValues(self):
        '''
        returns artUserPaintCtx value for selectedattroper option
        '''
        if self.controls.paintModeAdd.getValue():
            return self.PAINTMODE_ADD,self.intensityAdd
        if self.controls.paintModeScale.getValue():
            return self.PAINTMODE_SCALE,self.intensityScale
        if self.controls.paintModeSmooth.getValue():
            return self.PAINTMODE_SMOOTH,self.intensitySmooth
        return self.PAINTMODE_REPLACE,self.intensityReplace
        
    
    def configurePaintValues(self):
        '''
        sets paint tool values from UI
        '''    
        oper,pvalue = self.getPaintModeValues()
        cmds.ngSkinLayer(paintOperation=oper,paintIntensity=pvalue.get())

        if cmds.artUserPaintCtx(self.TOOL_PAINT,exists=True):
            # internally, use maya's "replace" brush with intensity of 1.0
            cmds.artUserPaintCtx(self.TOOL_PAINT,e=True,
                selectedattroper='absolute',
                value=1.0,
                opacity=1.0,
                brushfeedback=False,
                accopacity=False,
                stampProfile=self.getSelectedBrushShape())
            
        self.updateToTool()
        
    def storeIntensitySettings(self):
        '''
        stores intensity settings plugin-side
        '''
        
        cmds.ngSkinLayer(paintOperation=self.PAINTMODE_REPLACE,paintIntensity=self.intensityReplace.get())
        cmds.ngSkinLayer(paintOperation=self.PAINTMODE_ADD,paintIntensity=self.intensityAdd.get())
        cmds.ngSkinLayer(paintOperation=self.PAINTMODE_SCALE,paintIntensity=self.intensityScale.get())
        cmds.ngSkinLayer(paintOperation=self.PAINTMODE_SMOOTH,paintIntensity=self.intensitySmooth.get())


    def changeBrushRadius(self):            
        if cmds.artUserPaintCtx(self.TOOL_PAINT,exists=True):
            cmds.artUserPaintCtx(self.TOOL_PAINT,e=True,
                radius=self.controls.brushRadiusSlider.getValue())                                     

                
        
        
    def getSelectedBrushShape(self):
        for button,shape in zip(self.controls.brushShapeButtons,["gaussian","poly","solid","square"]):
            if cmds.symbolCheckBox(button,q=True,value=True):
                return shape

        return "solid"
    
    def selectIntensityModel(self):
        '''
        selects the right model to edit into intensity slider (each brush mode has it's own intensity variable)
        '''
        _, pvalue = self.getPaintModeValues()
        self.controls.intensitySlider.setModel(pvalue)
    
    def paintValuesChanged(self):
        '''
        brush UI change handler, called when user select new brush shape or intensity value
        '''
        self.selectIntensityModel()
        self.configurePaintValues()
            
    def brushButtonClicked(self,clickedIndex):
        '''
        handler for brush button click
        '''
        self.brushShape.set(clickedIndex)
        for index,button in enumerate(self.controls.brushShapeButtons):
            cmds.symbolCheckBox(button,e=True,value=index==clickedIndex) 
        self.paintValuesChanged()
        
    def createBrushShapeButtons(self):
        
        class ButtonClickHandler:
            def __init__(self,number,parent):
                self.number=number
                self.parent=parent
            def __call__(self,*args):
                self.parent.brushButtonClicked(self.number)
                
        newIcons = ['circleGaus.png','circlePoly.png','circleSolid.png','rect.png']
        oldIcons = ['circleGaus.xpm','circlePoly.xpm','circleSolid.xpm','rect.xpm']
        icons = newIcons if Utils.getMayaVersion()>=Utils.MAYA2011 else oldIcons
        for index,i in enumerate(icons):
            btn = cmds.symbolCheckBox(w=33,h=36,i=i,changeCommand=ButtonClickHandler(index,self),value=index==self.brushShape.get())
            self.controls.brushShapeButtons.append(btn)
            
    def createBrushSettingsGroup(self,parent):
        group = self.createUIGroup(parent, 'Brush Settings')

        self.createTitledRow(group, 'Brush Shape')
        brushWidth = 35;
        cmds.rowLayout(nc=4,cw4=[brushWidth,brushWidth,brushWidth,brushWidth])
        self.createBrushShapeButtons()
        cmds.setParent("..")
        
        def innerLayout():
            return cmds.rowColumnLayout( numberOfColumns=2,columnWidth=[(1,100),(2,100)])

        self.createTitledRow(group, 'Mode',innerContentConstructor=innerLayout)

        cmds.radioCollection()
        for index,i in enumerate(['Replace','Add','Scale','Smooth']):
            ctrl = self.controls.__dict__['paintMode'+i] = RadioButtonField(self.VAR_PREFIX+'paintMode'+i,defaultValue=1 if index==0 else 0,label=i)
            ctrl.changeCommand.addHandler(self.paintValuesChanged)
            
        
        self.controls.intensitySlider = FloatSliderField()
        self.createTitledRow(group, 'Intensity',self.controls.intensitySlider.create)
        self.controls.intensitySlider.onChange.addHandler(self.paintValuesChanged)

        self.controls.brushRadiusSlider = FloatSliderField(range=[0,30])
        self.controls.brushRadiusSlider.flexibleRange = True
        self.createTitledRow(group, 'Brush Radius',self.controls.brushRadiusSlider.create)
        self.controls.brushRadiusSlider.onChange.addHandler(self.changeBrushRadius)
        
    def createUI(self,parent):
        from ngSkinTools.ui.mainwindow import MainWindow
        
        LayerEvents.currentInfluenceChanged.addHandler(self.updateHighlight,parent)
        LayerEvents.layerAvailabilityChanged.addHandler(self.updateToTool,parent)
        MayaEvents.nodeSelectionChanged.addHandler(self.updateToTool, parent)
        MayaEvents.toolChanged.addHandler(self.updateToTool,parent)
        
        self.setTitle('Paint')

        def commandButtons():
            yield ('Mirror',MainWindow.getInstance().actions.mirrorWeights,'')
            yield ('Paint',self.execPaint,'')

        self.cmdLayout = self.createCommandLayout(commandButtons(), SkinToolsDocs.MIRRORWEIGHTS_INTERFACE)
        
        self.createBrushSettingsGroup(self.cmdLayout.innerLayout)

        self.storeIntensitySettings()
        self.selectIntensityModel()
        self.configurePaintValues()
        
        self.updateToTool()
        
        return self.cmdLayout.outerLayout

        
        
                        
                        