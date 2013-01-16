Introduction
=============

.. todo:: |pending|

.. todo:: brief overview of what plugin is meant for

NgSkinTools is a skinning plugin for Autodesk Maya



Quick Start
------------

Things to check for first 
~~~~~~~~~~~~~~~~~~~~~~~~~

To begin using ngSkinTools, you first need to have a skinCluster bound mesh. Either open one of your existing rigs, or load up a new mesh and bind it to joints using smooth bind (:guilabel:`Skin > Bind Skin > Smooth Bind`).

Typically, you might have other deformers in deformation chain after skin cluster - for example, poly smooth node; you might have to turn them off, it it's changing vertex order. In general, same rules apply here as with standard Paint Skin Weights Tool - if you can use it to paint weights on your mesh (which indicates that skinCluster node can be properly resolved from selected mesh, and vertex order is intact), ngSkinTools should behave properly as well.


Initialization
~~~~~~~~~~~~~~~

.. todo:: |pending|

When you open up ngSkinTools dialog and select your mesh, most of the UI will be disabled as plugin needs some more additional setup to be done before you can use all the features of ngSkinTools on your mesh. When you select the mesh, you should see something similar to this:

.. image:: _img/initialize.jpg

This shows the discovered mesh shape from your selection, and skin cluster that is attached to it. Click "Ini


