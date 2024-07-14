# MeshTally
  id=um

!alert note
`MeshTally` can only be added to problems when the input files contains a [OpenMCCellAverageProblem](OpenMCCellAverageProblem.md)
in the `[Problem]` block. Otherwise, attempting to add this tally will result in an error.

## Description

The `MeshTally` class wraps an OpenMC tally with an unstructured libMesh filter to directly enable tallying on an unstructured mesh.
There are two options when using a `MeshTally`:

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

!alert note
At present time, libMesh unstructured mesh tallies only support `collision` and `analog` estimators. Attempting to set `tally_estimator`
to `tracklength` will result in a warning; the `MeshTally` will then manually reset the estimator to `collision` to prevent OpenMC from
throwing an error.

## Example Input File Syntax

As an example, this `MeshTally` scores `kappa_fission` (the default tally score) on the unstructured mesh
found in `mesh_in.e` and stores it in a variable called `heat_source`. The `[Tallies]` block adds three
`MeshTally` classes, each one located at a different point specified in `mesh_translations` which corresponds
to the centroid of a fuel pebble.

!listing /tutorials/pebbles/openmc_um.i
  block=Tallies

!syntax parameters /Tallies/MeshTally

!syntax inputs /Tallies/MeshTally

!syntax children /Tallies/MeshTally
