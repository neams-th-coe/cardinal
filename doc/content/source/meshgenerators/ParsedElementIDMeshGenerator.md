# ParsedElementIDMeshGenerator


## Description

`ParsedElementIDMeshGenerator` is a mesh generator that wraps an existing input mesh and adds one or
more extra element integer id (eeid) to the mesh, where each eeids corresponds to user-defined values.
Clustering for mesh tally amalgamation, can benefit from it by using these eeid to lebel the cluster 
based on different heuristics.

Each additional eeid is:
- Named by an entry in the `extra_element_integer_names` list.
- Populated with values from a corresponding user provided values. 
- Skipped if a field with the same name already exists, 
in which case a warning is printed and the original values are preserved.

## Behavior
* Takes ownership of an input mesh (required).
* Adds named element integer fields to the mesh.
* Emits a warning if any requested field already exists and does not overwrite it.

## Warning

If an `extra_element_integer_names` entry already exists in the mesh, the generator **does not override** the values.
It will show a warning message and skip that field. This ensures existing mesh annotations are preserved.

## Example Input File
This example reads a mesh file (in this case mesh_in.e) by using [FileMeshGenerator](https://mooseframework.inl.gov/source/meshgenerators/FileMeshGenerator.html)
then the `ParsedElementIDMeshGenerator` adds three eeid **(a, b, c)** to the mesh. The values aginst eeid **(a, b, c)** will be **(-1, 44, 20)** through out the whole mesh.
```
[Mesh]
 [file_mesh_generator]
    type=FileMeshGenerator
    input=mesh_in.e
 []
  [add_eeid_block]
    type = ParsedElementIDMeshGenerator
    input = file_mesh_generator
    extra_element_integer_names = 'a b c'
    values = '-1 44 20'
  []
[]
```
