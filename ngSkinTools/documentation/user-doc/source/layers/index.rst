Skinning Layers
===============

Introduction
------------

Layers in weight painting bring pretty much the same benefits as they do in an image editing application: Instead of working on a single "canvas", you're able to have as many layers as you want, and this allows to:
	
	* Isolate parts of your work: paint character torso weights in one layer, arms in another; when you get back to torso, you know you won't mess with weights in other parts of the character
		
		image - torso weights + arm weights = finished weighting
		
	* Non destructive workflow: because you're working in different layers, you're always preserving what you've painted previously. 
	
		image: paint weights over -> delete new weights -> old weights visible again.
	
	* Non-linear workflow: no need to finish "background" before painting "foreground". This allows working more loosely, being able to get back and refine certain parts later.
		
		image: paint torso -> paint arm -> smooth torso -> smooth arm 
	
	* Apply operations per-layer, like weight smoothing or mirroring. This helps you work cleaner: you're mirroring just the part you're working on at the moment, and preserving non-symmetrical details on top in a different layer.
	
How It Works?
-------------

.. todo:: explain it works with standard skin cluster

Layered weight painting completely replaces Maya's build-in weight painting. The old tool does not go away, just
You use similar painting tools to those of tra

Thinking differently
--------------------

.. todo:: unfinished documentation

To really benefit from layers, you'll probably need to change the way you approach various skinning problems.

Throwing a bunch of weights into one layer, then use other layers just to tweak result is not optimal. Separate weighting into small sets of weights, then use masking to blend layers together.

Tips
-----
* Use just a few influences per layer - this helps being organized.
* Setup different layers for each logical part of your rig

.. todo:: unfinished documentation
 
