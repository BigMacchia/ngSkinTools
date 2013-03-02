from maya import OpenMaya as om
from maya import OpenMayaMPx
from ngSkinTools.utils import Utils, MessageException


class MeshDataExporter:
    
    def __init__(self):
        self.meshMObject = None
        self.transformMatrix = None
        
    def useSkinClusterInputMesh(self,skinCluster):
        '''
        sets skincluster's input mesh as source mesh
        '''
        skinClusterObject = Utils.getMObjectForNode(skinCluster)
        plug = om.MPlug(skinClusterObject,OpenMayaMPx.cvar.MPxDeformerNode_inputGeom)
        plug.selectAncestorLogicalIndex(0,OpenMayaMPx.cvar.MPxDeformerNode_input)
        self.meshMObject = plug.asMObject()
        
    def getWorldMatrix(self,nodeName):
        # get world transform matrix for given mesh transform node
        return Utils.getDagPathForNode(nodeName).inclusiveMatrix()
        
        #transformNode = Utils.getMObjectForNode(nodeName)         
        #nodeFn = om.MFnDependencyNode(transformNode)
        #matrixAttr = nodeFn.attribute("worldMatrix")
        #matrixPlug = om.MPlug(transformNode, matrixAttr).elementByLogicalIndex(0)
        #transform = om.MFnMatrixData(matrixPlug.asMObject()).transformation().asMatrix()
        #return transform
    
    def setTransformMatrixFromNode(self,transformNode):
        self.transformMatrix = self.getWorldMatrix(transformNode)
       
    def export(self):
        '''
        returns mesh triangles: first vertex list, then vertex ID list for each triangle;
        meshTransform (supplied as transform node name) is required to transform
        each vertex to world-space
        '''
 
        # get triangles for the mesh
        fnMesh = om.MFnMesh(self.meshMObject)
        counts = om.MIntArray()
        vertices = om.MIntArray()
        fnMesh.getTriangles(counts,vertices)
        idList = [id for id in Utils.mIter(vertices)]
        
        # get point values
        points = om.MPointArray()
        fnMesh.getPoints(points)
        pointList = []
        for p in Utils.mIter(points):
            p = p*self.transformMatrix
            pointList.extend((p.x,p.y,p.z))

        # return point values, id values            
        return pointList,idList 