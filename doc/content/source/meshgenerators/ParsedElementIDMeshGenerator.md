# ParsedElementIDMeshGenerator


## Description

`ParsedElementIDMeshGenerator` is a mesh generator that wraps an existing input mesh and adds one or
more extra element integer id (eeid) to the mesh, where each eeids corresponds to user-defined values.
Clustering for mesh tally amalgamation, can benefit from it by using these eeid to label the cluster 
based on different heuristics.

## Input Parameters:
- `extra_element_integer_names` list of the eeid names.
- `values` values for the eeid_names.
- `input` the mesh where we want to add the eeids.

## Warning

If an `extra_element_integer_names` entry already exists in the mesh, the generator **does not override** the values.
It will show a warning message and skip that field. This ensures existing mesh annotations are preserved.

## Example Input File
This example reads a mesh file (in this case mesh_in.e) by using [FileMeshGenerator](FileMeshGenerator.md)
then the `ParsedElementIDMeshGenerator` adds three eeid **(a, b, c)** to the mesh. The values aginst eeid **(a, b, c)** will be **(-1, 44, 20)** through out the whole mesh.

!listing tutorials/mesh_tally_amalgamation/clustering/threshold_heuristic.i
block=Mesh


!syntax parameters /Mesh/ParsedElementIDMeshGenerator
!syntax inputs /Mesh/ParsedElementIDMeshGenerator