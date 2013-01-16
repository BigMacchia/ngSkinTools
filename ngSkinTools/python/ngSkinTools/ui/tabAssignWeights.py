from maya import cmds
from ngSkinTools.utils import Utils, MessageException
from ngSkinTools.ui.basetab import BaseTab
from ngSkinTools.doclink import SkinToolsDocs
from ngSkinTools.ui.intensityslider import IntensitySlider
from ngSkinTools.ui.uiWrappers import IntField, CheckBoxField,\
    RadioButtonField
from ngSkinTools.ui.constants import Constants
from ngSkinTools.ui.softSelectionRow import SoftSelectionRow

     

class TabAssignWeights(BaseTab):
    '''
        Defines UI for Assign Weights operations.
    '''
    
    # prefix for preset variables for this tab
    VAR_ASSIGNWEIGHTS_PREFIX = 'ngSkinToolsAssignWeights_'
    
    VAR_CJ_PREFIX = VAR_ASSIGNWEIGHTS_PREFIX+'ClosestJoint'
    
    INFO_INTERSECTION_CHECK = 'when on, selects influence to which the closest path has the least amount of intersections'
    
    VAR_RIGID_PREFIX = VAR_ASSIGNWEIGHTS_PREFIX+'UnifyWeights'
    VAR_LW_PREFIX = VAR_ASSIGNWEIGHTS_PREFIX+'LimitWeights'
    
    def __init__(self):
        BaseTab.__init__(self)
        

    def createClosestJointGroup(self,layout):
        group = self.createUIGroup(layout,"From Closest Joint")

        # influence chooser group
        influenceFiltersForm = cmds.formLayout(parent=group)
        l = cmds.text(label='Influences to choose from: ')
        cmds.formLayout(influenceFiltersForm,e=True,attachForm=[(l,'left',0),(l,'top',0)],attachNone=[(l,'bottom'),(l,'right')])
        radios = cmds.columnLayout();
        cmds.radioCollection()
        cmds.formLayout(influenceFiltersForm,e=True,attachForm=[(radios,'left',Constants.MARGIN_BLOCK_INDENT),(radios,'right',0),(radios,'bottom',0)],attachControl=[(radios,'top',0,l)])
        RadioButtonField(self.VAR_CJ_PREFIX+'useAllInfluences',defaultValue=1,label='All influences available in skin cluster',
                         annotation='Use all influences present in skin cluster for closest joint  search')
        
        self.controls.rdioUseListerInfluences = RadioButtonField(self.VAR_CJ_PREFIX+'useSelectedInfluences',
                        defaultValue=0,label='Selected influences in lister',
                        annotation='Use only those influences that are selected in "List Influences" tab')
        
        cmds.setParent(group)
        
        self.controls.cjIntensity = IntensitySlider(
                'amount of effect to apply',
                self.VAR_CJ_PREFIX+'Intensity')
        self.createTitledRow(group, "Intensity",innerContentConstructor=self.controls.cjIntensity.create)
        
        self.controls.cjSoftSelection = SoftSelectionRow(self.VAR_CJ_PREFIX+'softSelection')
        self.controls.cjSoftSelection.create(group)

        cmds.setParent(group)
        cmds.rowLayout(nc=2,adjustableColumn=2,columnWidth2=[Constants.BUTTON_WIDTH_SMALL,50], columnAttach2=["both","both"],columnAlign2=["center","center"])
        BaseTab.createHelpButton(SkinToolsDocs.ASSIGNWEIGHTS_CLOSESTJOINT_INTERFACE)
        cmds.button(height=Constants.BUTTON_HEIGHT,label='Assign',command=lambda *args:self.execClosestJointAssign())
        
    def createMakeUnifyGroup(self,layout):
        group = self.createUIGroup(layout, 'Unify Weights')

        self.controls.rigidIntensity = IntensitySlider(
                'amount of effect to apply',
                self.VAR_RIGID_PREFIX+'Intensity')
        self.createTitledRow(group, "Intensity",innerContentConstructor=self.controls.rigidIntensity.create)

        row = self.createFixedTitledRow(group, "Clustering")
        self.controls.chkSingleClusterMode = CheckBoxField(self.VAR_RIGID_PREFIX+"singleClusterMode",
                    "Ignore separate shells or selection gaps", False, "When selected, whole selection is treated as one vertex cluster, regardless if vertices "+ 
                    " are in different shells or selection is not contiguous");

        
        self.controls.rigidSoftSelection = SoftSelectionRow(self.VAR_RIGID_PREFIX+'softSelection')
        self.controls.rigidSoftSelection.create(group)
        

        cmds.setParent(group)

        cmds.rowLayout(nc=2,adjustableColumn=2,columnWidth2=[Constants.BUTTON_WIDTH_SMALL,50], columnAttach2=["both","both"],columnAlign2=["center","center"])
        BaseTab.createHelpButton(SkinToolsDocs.ASSIGNWEIGHTS_MAKERIGID_INTERFACE)
        cmds.button(height=Constants.BUTTON_HEIGHT,label='Assign',command=lambda *args:self.execUnifyWeights())

    def createLimitWeightsGroup(self,layout):
        group = self.createUIGroup(layout, 'Limit Weights')

        cmds.rowLayout(parent=group,nc=2,columnWidth2=[Constants.MARGIN_COLUMN2,Constants.NUMBER_FIELD_WIDTH], columnAttach2=["both","both"],columnAlign2=["right","left"])
        cmds.text(label='Max Limit: ')
        self.controls.limitWeightsMaxInfluences = IntField(self.VAR_LW_PREFIX+'maxInfluences', minValue=1,maxValue=1000,step=1,
                defaultValue=3, annotation='Maximum amount of influences per vertex')

        self.controls.limitWeightsIntensity = IntensitySlider(
                'amount of effect to apply',
                self.VAR_LW_PREFIX+'Intensity')
        self.createTitledRow(group, "Intensity",innerContentConstructor=self.controls.limitWeightsIntensity.create)
        
        self.controls.limitWeightsSoftSelection = SoftSelectionRow(self.VAR_LW_PREFIX+'softSelection')
        self.controls.limitWeightsSoftSelection.create(group)

        cmds.setParent(group)

        cmds.rowLayout(nc=2,adjustableColumn=2,columnWidth2=[Constants.BUTTON_WIDTH_SMALL,50], columnAttach2=["both","both"],columnAlign2=["center","center"])
        BaseTab.createHelpButton(SkinToolsDocs.ASSIGNWEIGHTS_LIMITWEIGHTS_INTERFACE)
        cmds.button(height=Constants.BUTTON_HEIGHT,label='Assign',command=lambda *args:self.execLimitWeights())
        
    
    @Utils.visualErrorHandling
    def execAssignWeights(self,args):
        Utils.testPluginLoaded()
        #Utils.testVertexSelectionAvailable()
        cmds.ngAssignWeights(**args);
    
    @Utils.visualErrorHandling
    def execClosestJointAssign(self):
        '''
        runs 'assign from closest joint' operation
        '''
        args = {}
        args['bnj'] = True; # by nearest joint
        
            
        # restrict influence list?
        if self.controls.rdioUseListerInfluences.getValue():
            # add include joints
            selInfluences = self.parentWindow.targetUI.getSelectedInfluences();
            if len(selInfluences)==0:
                raise MessageException('no influences selected')
            
            args['ij'] = '/'.join(selInfluences);
        
        args['intensity'] = self.controls.cjIntensity.getIntensity()
        
        self.controls.cjSoftSelection.addToArgs(args)
        self.execAssignWeights(args)
        
    def execUnifyWeights(self):
        args = {}
        args['mr'] = True # make rigid
        args['intensity'] = self.controls.rigidIntensity.getIntensity()
        if self.controls.chkSingleClusterMode.getValue():
            args['singleCluster'] = True; 
        self.controls.rigidSoftSelection.addToArgs(args)
        self.execAssignWeights(args)

    def execLimitWeights(self):
        args = {}
        args['lw'] = True # limit weights
        args['intensity'] = self.controls.limitWeightsIntensity.getIntensity()
        self.controls.limitWeightsSoftSelection.addToArgs(args)
        self.execAssignWeights(args)
        
        
    def createUI(self,parent):
        self.setTitle('Assign Weights')
        result = self.createScrollLayout(parent=parent)        
        self.baseLayout = cmds.columnLayout(adjustableColumn=1)
        
        self.createClosestJointGroup(self.baseLayout)
        self.createMakeUnifyGroup(self.baseLayout)
        #self.createLimitWeightsGroup(self.baseLayout)
        
        return result
    