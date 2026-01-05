# BooleanComboClusteringUserObject

## Description

`BooleanComboClusteringUserObject` implements the overall mesh walking and clustering
process. It takes an expression as a logical function of different heuristic
based user objects. It evalutes the expression between two adjacent elements which share at least 
one common side. 

If the expression is true for those two elements then the `extra_integer` 
for those two elements are changed. If an element belongs to a cluster, the `extra_integer` is
set to be the same as the element id of the first element in that cluster.

## Example Input File

The following input block gives an example how `BooleanComboClusteringUserObject`
can implement two different [ThresholdHeuristicUserObject](ThresholdHeuristicUserObject.md) heuristics with
boolean logic. 

!listing /test/tests/userobjects/clustering/threshold_heuristic/example_input.i
block=UserObjects

!syntax parameters /UserObjects/BooleanComboClusteringUserObject
!syntax inputs /UserObjects/BooleanComboClusteringUserObject