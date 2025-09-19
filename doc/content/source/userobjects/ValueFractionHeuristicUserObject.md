# ValueFractionHeuristicUserObject

## Description

`ValueFractionHeuristicUserObject` implements the clustering heuristic if the variable values of two adjacent
elements are both either more/less than an upper/lower cut off of the metric variable.
The upper and lower cut off are calculated as,

upper_cut_off $= (1 -$ upper_fraction $)$ $ \times (S_{max} - S_{min}) + S_{min}$ $
lower_cut_off $= $ lower_fraction $ \times (S_{max} - S_{min}) + S_{min}$

where `upper_fraction` and `lower_fraction` are user defined input parameters and $S_{max}$ and
$S_{min}$ are maximum and minimum value of the `metric_variable`

## Example Input File

The following input block declares a `ValueFractionHeuristicUserObject` which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and cluster process is implemented.

!listing /test/tests/userobjects/clustering/value_range/example.i
block=UserObjects

!syntax parameters /UserObjects/ValueFractionHeuristicUserObject
!syntax inputs /UserObjects/ValueFractionHeuristicUserObject