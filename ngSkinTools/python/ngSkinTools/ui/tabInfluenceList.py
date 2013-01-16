from maya import cmds
from ngSkinTools.utils import Utils
from ngSkinTools.ui.basetab import BaseTab
from ngSkinTools.ui.constants import Constants
from ngSkinTools.ui.events import MayaEvents
from ngSkinTools.log import LoggerFactory

log = LoggerFactory.getLogger("tabInfluenceList")

class TabInfluenceList(BaseTab):
    '''
        Defines UI for Influence Lister operations.
    '''
    
    # prefix for preset variables for this tab
    VAR_RELAX_PREFIX = 'ngSkinToolsInflLister_'
    
    LIST_METHOD_CONTAINED_WEIGHTS = 'By Contained Weights'
    LIST_METHOD_CLOSEST_JOINT = 'By Closest Joint'
    
    INFO_NAME_FILTER = 'type influence name fragment (e.g. "spine") to filter items by name. separate more names by space to get multiple results (e.g. "spine neck")'
    
    def __init__(self):
        BaseTab.__init__(self)
        self.listedInfluences = []
        
        

    def filterByName(self,name):
        '''
        returns true when influence with given name should appear on the list
            * true if filter does not exist;
            * true if given name matches entered filter
            * false for the rest
        '''
        filter = cmds.textField(self.controls.nameFilter,q=True,text=True)
        filter = filter.strip()
        if filter=='':
            return True
        
        filter = filter.lower().split(" ")
        
        for f in filter:
            if f!='' and f in name.lower():
                return True
        return False
       
    def setDisplayedItems(self,items,column2=None):
        '''
        use this method as the only entry point to set items into influence list
        '''
        if items is None:
            items = []
        
        self.listedInfluences = []

        cmds.textScrollList (self.controls.inflList,e=True,removeAll=True)
        for index,i in enumerate(items):
            if not self.filterByName(i):
                continue
            self.listedInfluences.append(i)
            
            
            listItemName = i
            if column2 is not None:
                listItemName += "  " + column2[index]
            cmds.textScrollList (self.controls.inflList,e=True,append=listItemName)

    def breakToColumns(self,itemList,numColumns):
        '''
        breaks given list into number of columns, e.g.
        [1,2,3,4,5,6,7,8,9] to three columns: ( [1,4,7], [2,5,8], [3,6,9])
        '''
        assert (len(itemList) % numColumns==0)
        result = []
        for i in xrange(numColumns):
            result.append([])
            
        for index,i in enumerate(itemList):
            result[index % numColumns].append(i);
        return result
        
        
            
    def listFromSelection(self):
        '''
        list influences based on current vertex selection
        '''
        includeWeightingInfo = True
        

        if not Utils.isVertexSelectionAvailable():
            self.setDisplayedItems([])
            return
        
        # TODO: this will result in error when plugin is not loaded (eat the error or warn about plugin not loaded)
        options = {}
        method = self.getListingMethod() 
        if method==self.LIST_METHOD_CLOSEST_JOINT:
            options['listByClosestJoint'] = True
        elif method==self.LIST_METHOD_CONTAINED_WEIGHTS:
            options['listByWeights'] = True
            
#        options['sortbyTotalWeights'] = True
#        options['includeWeightingInfo'] = includeWeightingInfo
        items = cmds.ngListInfluences(**options)

        if not includeWeightingInfo:
            self.setDisplayedItems(items)
            return
        items,column2 = self.breakToColumns(items, 2)
        
        # for weight listing, display weighting in percentage
        if method==self.LIST_METHOD_CONTAINED_WEIGHTS:
            column2 = map(lambda a: "%11.2f%%" % (float(a)*100),column2)

        # for closest joint, just round float a bit
        if method==self.LIST_METHOD_CLOSEST_JOINT:
            column2 = map(lambda a: "%11.2f" % float(a),column2)
            
        self.setDisplayedItems(items,column2)
            
    
    def updateInfluenceList(self):
        '''
        gets called whenever influence list should be updated
        '''
        self.listFromSelection()
        
    
    @Utils.visualErrorHandling
    def selectPaintWeightsInfluence(self,infl):
        '''
        tries to select influence (provided as string) in current maya's paint weights context and UI
        if skin paint context is not available, nothing happens
        '''
        if not Utils.isCurrentlyPaintingWeights():
            return
        
        # influence name can come in any form ('joint3', 'joint2|joint3', 'joint1|joint2|joint3')
        # get the absolute shortest possible (but still unique) and longest
        try:
            longName = cmds.ls(infl,l=True)[0]
            shortName = cmds.ls(longName,l=False)[0]

            log.info("selecting in paint weights: influence %s" % str(infl))
        
            # try to work around with the mess in the earlier versions of 
            # maya's paint weights UI:
            if Utils.getMayaVersion()<Utils.MAYA2011:
                itemName = Utils.mel('artAttrSkinShortName("%s")'%shortName)
                Utils.mel('artSkinSelectInfluence("artAttrSkinPaintCtx","%s","%s");' % (shortName,itemName));
            else:
                Utils.mel('artSkinSelectInfluence("artAttrSkinPaintCtx","%s");' % shortName);
                
            # show paint weights interface
            cmds.toolPropertyWindow()
        except:
            # problems listing given influence.. just die here
            Utils.displayError('problem selecting influence %s' % infl)
    
    
    def updateMenuItems(self):
        # update menu items
        infl = self.getSelectedInfluence()
        selectionAvailable = infl!=None

        # enabled/disabled state
        cmds.menuItem(self.controls.menuPinInfluence,e=True,enable=selectionAvailable)
        cmds.menuItem(self.controls.menuPaintWeights,e=True,enable=selectionAvailable)
        
        # captions
        if infl is not None:
            cmds.menuItem(self.controls.menuPaintWeights,e=True,label='Paint Weights on %s' % infl)
        
    def getSelectedInfluence(self):
        '''
        returns path to selected influence; 
        path is partial, like the one displayed in influence list
        if nothing is selected, returns None  
        '''
        
        influences = self.getSelectedInfluences()
        if len(influences)==0:
            return None
        # on multiple selection just go for the first one
        return influences[0]
    
    def getSelectedInfluences(self):
        '''
        gets all selected influences as string array
        '''
        result = []
        influences = cmds.textScrollList(self.controls.inflList,q=True,sii=True)
        if influences is not None:
            for i in influences:
                # indexes from textScrollList are returned 1-based
                result.append(self.listedInfluences[i-1])
        return result
    
        
    def doInfluenceListItemSelected(self,*args):
        '''
        gui handler for item selection in influence list
        '''
        
        self.updateMenuItems()
        influence = self.getSelectedInfluence() 
        if influence != None:
            self.selectPaintWeightsInfluence(influence)
    
    def doInflListPaintWeights(self,*args):
        '''
        opens paint weights tool and paints on locally selected influence  
        '''
        if not Utils.isCurrentlyPaintingWeights():
            cmds.ArtPaintSkinWeightsTool()
            
        self.doInfluenceListItemSelected()
        
    def createInflListContextMenu(self):
        list = self.controls.inflList
        self.controls.inflListMenu = cmds.popupMenu( parent=list )
        self.controls.menuPinInfluence = cmds.menuItem(label='Pin Influence',enable=False)
        self.controls.menuPaintWeights = cmds.menuItem(label='Paint Weights',enable=False,command=self.doInflListPaintWeights)

        cmds.menuItem( divider=True )
        cmds.radioMenuItemCollection()
        self.controls.menuNoLockFilter = cmds.menuItem( label='Show Both Locked and Unlocked Influences', radioButton=True )
        self.controls.menuHideLocked = cmds.menuItem( label='Hide Locked Influences', radioButton=False )
        self.controls.menuHideUnlocked = cmds.menuItem( label='Hide Unlocked Influences', radioButton=False )
        
    def doNameFilterChanged(self,*args):
        '''
        gets executed when name filter changes
        '''
        
        self.updateInfluenceList()
        
    def createNameFilterRow(self):
        cmds.rowLayout(nc=2,adjustableColumn=2,columnWidth2=[Constants.MARGIN_COLUMN2,50], columnAttach2=["both","both"],columnAlign2=["right","left"])
        cmds.text(label='Name Filter: ')
        self.controls.nameFilter = cmds.textField( 
                text='',
                annotation=self.INFO_NAME_FILTER,changeCommand=self.doNameFilterChanged,alwaysInvokeEnterCommandOnReturn=True)
        
    def getListingMethod(self):
        return cmds.optionMenu(self.controls.cmbListingMethod,q=True,value=True);
        
    def createListingMethodRow(self):
        cmds.setParent(self.baseLayout)
        cmds.rowLayout(nc=2,adjustableColumn=2,columnWidth2=[Constants.MARGIN_COLUMN2,50], columnAttach2=["both","both"],columnAlign2=["right","left"])
        
        cmds.text(label='Listing Method:')

        self.controls.cmbListingMethod = cmds.optionMenu(changeCommand=lambda *args: self.updateInfluenceList())
        cmds.menuItem(label=self.LIST_METHOD_CONTAINED_WEIGHTS)
        cmds.menuItem(label=self.LIST_METHOD_CLOSEST_JOINT)
        
            
    def createUI(self,parent):
        MayaEvents.nodeSelectionChanged.addHandler(self.updateInfluenceList,parent)

        self.setTitle('Influence Lister')
        BaseTab.createUI(self, parent)
        
        self.createListingMethodRow();

        cmds.setParent(self.baseLayout)
        self.createNameFilterRow()

        cmds.setParent(self.baseLayout)
        self.controls.inflList = cmds.textScrollList(numberOfRows=8,
                allowMultiSelection=True,
                selectCommand=self.doInfluenceListItemSelected,
                doubleClickCommand=self.doInflListPaintWeights)
        
        cmds.button('reload',command=lambda *args:self.updateInfluenceList())
        self.createInflListContextMenu()
        
        return self.baseLayout
    