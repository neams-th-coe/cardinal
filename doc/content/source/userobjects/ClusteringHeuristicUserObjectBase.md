# ClusteringHeuristicUserObjectBase

## Description

`ClusteringHeuristicUserObjectBase` works as the base class in Cardinal for identifying clusters in mesh results, where it may be beneficial to apply
mesh-tally amalgamation both in post processing or on the fly. It hosts the functionality to implement any clustering heuristics.

The `ClusteringHeuristicUserObjectBase` interacts with the [`AuxiliarySystem`] (AuxKernels/index.md) of the simulation.

Currently, the variable type must be `CONSTANT MONOMIAL`, since no quadrature point integration is performed
when retrieving values from the auxiliary system. Any `UserObject` derived from `ClusteringHeuristicUserObjectBase` 
must override the `evaluate()` method, where the specific clustering heuristic / logic is implemented.

