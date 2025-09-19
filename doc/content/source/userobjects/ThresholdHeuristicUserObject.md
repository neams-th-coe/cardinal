# ThresholdHeuristicUserObject

## Description

`ThresholdHeuristicUserObject` implements the clustering
heuristic if their scores are more (default) or less than a `threshold`
value provided in the input files. The operation will be flipped if the 
input argument `cluster_if_above_threshold` is false.

## Example Input File

The following input block declares a `ThresholdHeuristicUserObject` which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and cluster process is implemented.

!listing /test/tests/userobjects/clustering/threshold_heuristic/example_input.i
block=UserObjects

!syntax parameters /UserObjects/ThresholdHeuristicUserObject
!syntax inputs /UserObjects/ThresholdHeuristicUserObject