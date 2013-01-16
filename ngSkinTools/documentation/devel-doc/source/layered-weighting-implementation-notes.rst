Layered Weighting Implementation Notes
--------------------------------------

SkinLayer
~~~~~~~~~

* SkinLayer identifies influences by logical index: the reason for that is that they're unique, removing an influence in skin cluster won't shift
  IDs for each influence

ngSkinColorDisplayNode
~~~~~~~~~~~~~~~~~~~~~~

* undoable/redoable ngSkinColorDisplay node insertion/deletion
	* insert node before paint mode enters
	* delete node when paint exits
	* when updating mesh, layer manager is not inserting node, but:
		* finds the node in graph (right before mesh node)
		* reuses color node if handle is valid