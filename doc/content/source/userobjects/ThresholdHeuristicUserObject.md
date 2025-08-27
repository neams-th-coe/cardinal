# ThresholdHeuristicUserObject

## Description

`ThresholdHeuristicUserObejct` inherits from the ClusteringHeuristicUserObjectBase. It implements the clustering
heuristic if their scores are more (default) or less than a `threshold`
value provided in the input files. The operation will be flipped if the 
input argument `cluster_if_above_threshold` is false. 

!alert! warning
**ThresholdHeuristicUserObejct** doesn't implement the clustering and mesh walking process.
!alert-end!

The following input block declares a **ThresholdHeuristicUserObejct** which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and cluster process is implemented.

!listing tutorials/clustering/threshold_heuristic/example_input.i
block=UserObjects/threhsold

!syntax parameters /UserObject/ThresholdHeuristicUserObject
!syntax inputs /UserObject/ThresholdHeuristicUserObject