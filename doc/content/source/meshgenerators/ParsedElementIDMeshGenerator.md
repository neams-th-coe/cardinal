# ParsedElementIDMeshGenerator


## Description

`ParsedElementIDMeshGenerator` is a mesh generator that wraps an existing input mesh and adds one or
more extra element integer id (eeid) to the mesh, where each eeids corresponds to user-defined values.
Clustering for mesh tally amalgamation, can benefit from it by using these eeid to label the cluster 
based on different heuristics.

!alert! warning
If an `extra_element_integer_names` entry already exists in the mesh, the generator +will not+ override the values.
It will show a warning message and skip that field.
This ensures existing mesh annotations are preserved.
!alert-end!

## Example Input File

This example reads a mesh file (in this case mesh_in.e) by using a [FileMeshGenerator](FileMeshGenerator.md),
then the `ParsedElementIDMeshGenerator` adds three eeid +(a, b, c)+ to the mesh. The values of eeid +(a, b, c)+ will be +(-1, 44, 20)+ throughout the whole mesh.

!listing tutorials/mesh_tally_amalgamation/clustering/threshold_heuristic.i
block=Mesh


!syntax parameters /Mesh/ParsedElementIDMeshGenerator
!syntax inputs /Mesh/ParsedElementIDMeshGenerator