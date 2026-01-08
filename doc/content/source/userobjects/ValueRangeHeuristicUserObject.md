# ValueRangeHeuristicUserObject

## Description

`ValueRangeHeuristicUserObject` implements the clustering heuristic if the scores of two adjacent
elements are both within a range. The range is calculated as

\being{equation}
S_{\text{lower}} = (1 - \text{tolerance_percentage}) \times \text{value} 
\end{equation}

\begin{equation}
S_{\text{upper}} = (1 + \text{tolerance_percentage}) \times \text{value}
\end{equation}

where `value` and `tolerance_percentage`  are user-provided inputs.

## Example Input File

The following input block declares a `ValueRangeHeuristicUserObject` which
is then used by [BooleanComboClusteringUserObject](BooleanComboClusteringUserObject.md)
where the mesh walking and clustering process is implemented.

!listing /test/tests/userobjects/clustering/value_range/example.i
block=UserObjects

!syntax parameters /UserObjects/ValueRangeHeuristicUserObject
!syntax inputs /UserObjects/ValueRangeHeuristicUserObject