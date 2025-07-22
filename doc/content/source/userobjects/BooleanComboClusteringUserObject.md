# BooleanComboClusteringUserObject

## Description

`BooleanComboClusteringUserObject` implements the over all mesh walking and clustering
process. It takes an expression as a logical function of different heuristic
based user objects. It evalutes the expression between two adjacent elements which shares at least 
one common side. 

If the expression is true for those two elements then the `extra_integer` 
for those two elements are changed. If an element belongs to a cluster, the `extra_integer` is 
set same as the element id of the first element in that cluster.

!listing tutorials/clustering/threshold_heuristic/example_input.i
block=UserObjects/boolean_combo

!syntax parameters /UserObject/BooleanComboClusteringUserObject
!syntax inputs /UserObject/BooleanComboClusteringUserObject