# ValueDifferenceHeuristicUserObject

## Description

`ValueDifferenceHeuristicUserObject` implements the clustering heuristic if the relative
difference of the variable between two adjacent elements are less than a tolerance limit.

## Example Input File

The following input block declares a `ValueDifferenceHeuristicUserObject` which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and cluster process is implemented.

!listing /test/tests/userobjects/clustering/value_difference/example.i
block=UserObjects/value_diff

!syntax parameters /UserObjects/ValueDifferenceHeuristicUserObject
!syntax inputs /UserObjects/ValueDifferenceHeuristicUserObject