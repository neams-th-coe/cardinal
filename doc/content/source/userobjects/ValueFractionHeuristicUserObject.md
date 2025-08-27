# ValueFractionHeuristicUserObejct

## Description

`ValueFractionHeuristicUserObejct` inherits from the [ClusteringHeuristicUserObjectBase](ClusteringHeuristicUserObjectBase.md).
It implements the clustering heuristic if the scores of two adjacent
elements are both either more/less than a upper/lower cut off of the metric variable.
The upper and lower cut off are calculated as,

upper_cut_off $= (1 -$ upper_fraction $)$ $ \times (S_{max} - S_{min}) + S_{min}$ $
lower_cut_off $= $ lower_fraction $ \times (S_{max} - S_{min}) + S_{min}$

where `upper_fraction` and `lower_fraction` are user defined input parameters and $S_{max}$ and
$S_{min}$ are maximum and minimum value of the `metric_variable`

!alert! warning
`ValueFractionHeuristicUserObejct` doesn't implement the clustering and mesh walking process. Mesh walking and the clustering process are implemented
by the [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
!alert-end!

The following input block declares a `ValueFractionHeuristicUserObejct` which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and cluster process is implemented.

!listing tutorials/clustering/value_range/example.i
block=UserObjects/value_fraction

!syntax parameters /UserObject/ValueFractionHeuristicUserObejct
!syntax inputs /UserObject/ValueFractionHeuristicUserObejct