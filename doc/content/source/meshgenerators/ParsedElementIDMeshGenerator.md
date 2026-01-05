# ParsedElementIDMeshGenerator


## Description

`ParsedElementIDMeshGenerator` is a mesh generator that wraps an existing input mesh and adds one or
more [!ac](EEIID) to the mesh. Clustering for mesh tally amalgamation uses 
these [!ac](EEIID) to label the cluster based on different heuristics.

!alert! warning
If an `extra_element_integer_names` entry already exists in the mesh, the generator +will not+ override the values.
It will throw an error. This ensures existing mesh annotations are preserved.
!alert-end!

## Example Input File

This example reads a mesh (in this case `generated_mesh`) created by a [GeneratedMeshGenerator](GeneratedMeshGenerator.md),
then the `ParsedElementIDMeshGenerator` adds one [!ac](EEIID) `threshold_heuristic` to the mesh.
The value of [!ac](EEIID) `threshold_heuristic` will be `-1` throughout the whole mesh.

!listing /test/tests/userobjects/clustering/threshold_heuristic/example_input.i
block=Mesh


!syntax parameters /Mesh/ParsedElementIDMeshGenerator
!syntax inputs /Mesh/ParsedElementIDMeshGenerator