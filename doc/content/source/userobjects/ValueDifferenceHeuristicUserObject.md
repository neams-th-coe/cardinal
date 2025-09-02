# ValueDifferenceHeuristicUserObject

## Description

`ValueDifferenceHeuristicUserObject` inherits from the `ClusteringHeuristicUserObjectBase`.
It implements the clustering heuristic if the relative difference between scores of two adjacent
elements are less than a tolerance limit.

!alert! warning
`ValueDifferenceHeuristicUserObject` doesn't implement the clustering and mesh walking process. Mesh walking and the clustering process are implemented
by the [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
!alert-end!

The following input block declares a `ValueDifferenceHeuristicUserObject` which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and cluster process is implemented.

!listing /test/tests/userobjects/clustering/value_difference/example.i
block=UserObjects/value_diff

!syntax parameters /UserObjects/ValueDifferenceHeuristicUserObject
!syntax inputs /UserObjects/ValueDifferenceHeuristicUserObject