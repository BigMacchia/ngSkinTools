Troubleshooting
===============

General Usage
~~~~~~~~~~~~~~~~~~~~~~

.. rubric:: Relax command is not doing anything. I select vertices of skinned mesh, hit relax, nothing happens.

If history of your mesh contains several skin clusters, internal plugin engine might not properly recognize which skin cluster to operate on, and might be modifying the wrong skin cluster.

.. rubric:: Navigating in the scene is slower with ngSkinTools user interface open

ngSkinTools is updating parts of it's UI after every selection change, as well after undo and redo. There's no other workaround but just closing the UI if you're not using it, and opening when you need it again.

.. rubric:: Does ngSkinTools support skinning of NURBS surfaces and curves?

No. The only focus at the moment is mesh surfaces, as it's the most common use of skin clusters.

	
	
Installation & Other Technical Notes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
.. rubric:: ngSkinTools plugin fails to load due to conflict with another plugin

If you encounter node ID conflict, the problem is probably the developer of the other plugin. For our needs, we use IDs exclusively provided to us by Autodesk (range 0x00115A80 - 0x00115ABF).
	
