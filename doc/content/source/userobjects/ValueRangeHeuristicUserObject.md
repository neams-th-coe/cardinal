# ValueRangeHeuristicUserObject

## Description

`ValueRangeHeuristicUserObject` inherits from the `ClusteringHeuristicUserObjectBase`.
It implements the clustering heuristic if the scores of two adjacent
elements are both with in a range. The range is calculated as

$$S_{lower} = (1-tol) \times value $$
$$S_{upper} = (1+tol) \times value $$

where `value` and `_tolerance_percentage`  are user provided inputs.


!alert! warning
`ValueRangeHeuristicUserObject` doesn't implement the clustering and mesh walking process.  Mesh walking and the clustering process are implemented
by the [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
!alert-end!

The following input block declares a `ValueRangeHeuristicUserObject` which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and cluster process is implemented.

!listing /test/tests/userobjects/clustering/value_range/example.i
block=UserObjects

!syntax parameters /UserObjects/ValueRangeHeuristicUserObject
!syntax inputs /UserObjects/ValueRangeHeuristicUserObject