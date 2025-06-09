# ClusteringUserObject

## Description

ClusteringUserObject works as the base class for clustering in Cardinal for indentifying clusters, where it could be beneficial to apply
mesh-tally amalgamation both in post processing or on the fly. ClusteringUserObject class has been created, inheriting from 
[GeneralUserObject](https://mooseframework.inl.gov/docs/doxygen/moose/classGeneralUserObject.html).
The ClusteringUserObject interacts with the _auxiliary_system of the simulation. It iterates over 
the mesh elements and compares them using a specified variable, `metric_var`, which must belong to 
the [AuxVariables System](https://cardinal.cels.anl.gov/syntax/AuxVariables/index.html). This enables the use of values
computed by indicators. Currently, the element 
type must be CONSTANT MONOMIAL, since no quadrature point integration is performed
when retrieving values from the auxiliary system. If two elements are determined to
belong to the same cluster, their extra element integer IDs are unified by setting them
to the element_id of the first element in that cluster. Any UserObject derived from ClusteringUserObject 
must override the `belongsToCluster()` method, where the specific clustering heuristic / logic is implemented.

