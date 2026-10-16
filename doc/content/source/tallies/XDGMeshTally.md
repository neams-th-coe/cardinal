# XDGMeshTally
  id=xdg_um

!alert note
To use an `XDGMeshTally`, you must first build Cardinal with support for the [!ac](XDG) library.
See [the build instructions](start.md) for more information.

## Description

The `XDGMeshTally` class wraps an OpenMC tally with an unstructured mesh filter based on the [!ac](XDG)
backend, which yields significant performance improvements compared to [MeshTally](MeshTally.md).
`XDGMeshTally` supports the use of `tracklength` estimators, unlike [MeshTally](MeshTally.md) which only
supports `collision` and `analog` estimators. This comes with a series of limitations - at present,
`XDGMeshTally` does not support:

- Adaptive mesh refinement;
- Meshes with more than one element type;
- Meshes composed of elements that aren't TET4s or HEX8s;
- Block restriction.

Similar to a [MeshTally](MeshTally.md), you can use an `XDGMeshTally` in two different ways:

- Do nothing, in which case OpenMC will tally on the `[Mesh]`
- Specify a `mesh_template`, which provides a path to a mesh file

For the `mesh_template` option, it is possible
to translate the same mesh to multiple locations in the OpenMC geometry
(while only taking up the memory needed to store a single mesh) using
the `mesh_translations` or `mesh_translations_file` parameters provided by
the [tallies block](AddTallyAction.md). This is a useful feature for
geometries that consist of many repeated geometry units, such as pebble bed and pin fuel
systems.

!alert note
At present, unstructured mesh tallies are copied directly to the `[Mesh]` (without
doing any type of nearest-node lookup).
Suppose the mesh template consists of a mesh for a pincell with $N$ elements
that you have translated to 3 different locations, giving a total of $3N$ tally
bins. Because a direct copy is used to transfer the mesh tally results to the `[Mesh]`,
the first $3N$ elements in the `[Mesh]` must exactly match the $3N$ elements in
the mesh tally (except for a possible mesh scaling, as described in [OpenMCCellAverageProblem.md#scaling]).
This equivalence is required for the direct copy to be accurate - otherwise, the
mesh tally results would be transferred to incorrect regions of space.

!include scores_triggers.md

## Example Input File Syntax

As an example, this `XDGMeshTally` scores `kappa_fission` (the default tally score) on the unstructured mesh
found in `hex_cube.e` and stores it in a variable called `kappa_fission`. The `[Tallies]` block adds three
`XDGMeshTally` objects, each one located at a different point specified in `mesh_translations` which corresponds
to the centroid of a fissile pebble.

!listing /test/tests/neutronics/mesh_tally/xdg_multiple_meshes_global_trl.i
  block=Problem

!syntax parameters /Problem/Tallies/XDGMeshTally

!syntax inputs /Problem/Tallies/XDGMeshTally
