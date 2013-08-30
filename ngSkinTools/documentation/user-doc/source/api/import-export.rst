Import/Export
=============

.. py:module:: ngSkinTools.importExport

Import/export API consists of:
	* Data objects: :class:`LayerData`, :class:`Layer`, and :class:`Influence`; 
	  data objects are intermediate objects between exporters, importers and actual 
	  skin layer data in layers node;
	* Importers are used to load LayerData object from external format, e.g. XML or JSON
	* Similarly, exporters serialize LayerData objects into external formats.


For example, to import layers from XML into mesh you'd have to do the following:

.. code-block:: python

	from ngSkinTools.importExport import XmlImporter

	# create layers from	
	importer = XmlImporter()
	# read file contents here using one of your prefered methods
	xml = loadFileData('path/to/my.xml') 
	# load layer data
	data = importer.process(xml)
	# set that into given mesh
	data.saveTo('skinnedMesh')


To export, the process is reversed - we load data from skinned mesh (which should
have skin layers initialized for it) and save it to file:

.. code-block:: python

    from ngSkinTools.importExport import LayerData,XmlExporter

    data = LayerData()
    data.loadFrom('skinnedMesh')
    exporter = XmlExporter()
    xml = exporter.process(data)
    saveXmlToFile(xml,'path/to/my.xml')
    
Note that neither importers nor exporters work with files directly - instead,
they use/produce content as strings.

Having this modular approach instead allows for more usage paths other than "import from file" and
"export to file". Not only it allows plugging in new importers/exporters; instead loading data
from external source, it can be created on the fly; loaded layer data can be manipulated and saved
back to mesh; it's even possible to convert from one format to another!  
	
To create layer data object, do one of the following:
    * import with an Importer:
      
      .. code-block:: python
      	
		XmlImporter().process(xml)
    
    * load from a skinned mesh:
    
      .. code-block:: python
	      
	      data = LayerData()
	      data.loadFrom('rig|mesh|bodyMesh')
      
    * construct from scratch:
      
      .. code-block:: python
        
        data = LayerData()
        layer = Layer()
        # set layer properties
        influence = Influence()
        # set influence properties
        layer.addInfluence(influence)
        data.addLayer(layer)

:class:`LayerData` is an ordinary python object, containing a list of layers, which in turn 
contains a list of influences. For properties of each object, see below documentation. 
For more usage reference, look into implementations of XML/JSON importers/exporters.
	
.. autoclass:: ngSkinTools.importExport.LayerData
	:member-order: groupwise
	:members:

.. autoclass:: ngSkinTools.importExport.Layer
	:members:
	
	.. py:attribute:: name
	
		layer name. Default value: None; set/use as any python string.
		
	.. py:attribute:: opacity
	
		layer opacity. Defaults to 0.0. Set to float value between 0.0 and 1.0
		
	.. py:attribute:: enabled
	
		layer on/off flag. Default value is False. Set to True or False.
		
	.. py:attribute:: influences
	
		list of :class:`Influence` objects.
		
	.. py:attribute:: mask
	
		layer mask: list of floats. Set to None for uninitialized mask,
		or to float list, containing as many values as there are vertices
		in a target mesh.
	
.. autoclass:: ngSkinTools.importExport.Influence
	:members:

	.. py:attribute:: weights
	
		vertex weights for this influence. Set to float list, containing 
		as many values as there are vertices in a target mesh.

	.. py:attribute:: influenceName
	
		Full path of the influence in the scene. Required value when importing
		data back into skin cluster, as influences are associated by name in 
		current implementation. 
		
	.. py:attribute:: logicalIndex
	
		Logical index for this influence in a skin cluster. Not required for
		import and only provided in export as a reference.
		
