from maya import OpenMaya as om
from maya import OpenMayaMPx
from ngSkinTools.utils import Utils, MessageException


class MeshDataExporter:
    
    def __init__(self):
        self.meshMObject = None
        
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
        transformNode = Utils.getMObjectForNode(nodeName)         
        nodeFn = om.MFnDependencyNode(transformNode)
        matrixAttr = nodeFn.attribute("worldMatrix")
        matrixPlug = om.MPlug(transformNode, matrixAttr).elementByLogicalIndex(0)
        transform = om.MFnMatrixData(matrixPlug.asMObject()).transformation().asMatrix()
        return transform
        
    def exportMeshTriangles(self,meshTransform):
        '''
        returns mesh triangles: first vertex list, then vertex ID list for each triangle;
        meshTransform (supplied as transform node name) is required to transform
        each vertex to world-space
        '''
        
        transform = self.getWorldMatrix(meshTransform)

        
        for pt in (om.MPoint(0,0,0),om.MPoint(1,1,1)):
            pt = pt*transform
            print "%.3f %.3f %.3f" % (pt.x,pt.y,pt.z)

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
            p = p*transform
            pointList.extend((p.x,p.y,p.z))

        # return point values, id values            
        return pointList,idList 