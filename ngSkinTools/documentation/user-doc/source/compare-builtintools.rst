Comparision with Built-in Maya Tools
====================================

Influence Locking
-----------------

Influence locking feature (in paint weights mode) does not exist in ngSkinTools for various reasons. It was decided that for what the feature is for - isoliating some influences while working on the others - skinning layers do a much better job. So the whole workflow is reversed - instead of selecting joints you don't want to affect (which often is, "all of the joints except selected few"), you create a fresh layer and include only those influences you want to adjust.