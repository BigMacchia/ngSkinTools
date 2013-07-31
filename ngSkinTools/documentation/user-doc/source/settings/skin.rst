Selected Skin Settings
======================


This settings group applies settings to currently selected skin mesh. Layers should be initialized, otherwise UI will be disabled.


1. **Use maximum influences per vertex limit**: a post-processing filter that takes place just before writting weights into the skinCluster. For each vertex, number of influences
   is reduced to the selected limit. This happens transparently in the background without affecting contents of weights painted in layers.   
	
	+ ``Influence limit`` - maximum number of influences that can affect a vertex.