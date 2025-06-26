# ClusteringUserObject

## Description

`ClusteringUserObject` works as the base class in Cardinal for identifying clusters in mesh results, where it may be beneficial to apply
mesh-tally amalgamation both in post processing or on the fly. It implements the mesh walking process. This base class assumes that only active
elements, sharing at least one common side can be clustered together based on heuristics that will be implemented 
in user objects derived from this class. 

The `ClusteringUserObject` interacts with the [`AuxiliarySystem`] (AuxKernels/index.md) of the simulation. It iterates over 
the mesh elements and compares their metric variable value using different heuristics.

Currently, the variable type must be `CONSTANT MONOMIAL`, since no quadrature point integration is performed
when retrieving values from the auxiliary system. If two elements are determined to
belong to the same cluster, their extra element integer IDs are unified by setting them
to the element_id of the first element in that cluster. Any `UserObject` derived from `ClusteringUserObject` 
must override the `belongsToCluster()` method, where the specific clustering heuristic / logic is implemented.

