Weights Relax Limitations
=========================

These are known issues and limitations of current weights relax implementation:

* When relaxing huge selection of vertices, which is currently weighted to a lot of joints
  (example, 10000 vertices, weighted to 50 joints), relax operation takes very long. It only applies
  to situations where selected vertices are *weighted* to those 50 joints.
  
  Possible solution: relaxing in parts. Relax performs best when selection contains weights on 3-5 joints.
  