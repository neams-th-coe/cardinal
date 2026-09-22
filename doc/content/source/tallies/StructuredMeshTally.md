# StructuredMeshTally
  id=ssm

!alert note
Structured mesh tallies are not currently supported when running neutronics with OpenMC's
random ray solver.

## Description

The `StructuredMeshTally` class wraps an OpenMC tally with a **structured** (regular or
rectilinear) mesh spatial filter. Unlike the [MeshTally](MeshTally.md), which tallies on an
unstructured mesh that must match the `[Mesh]`, a `StructuredMeshTally` generates both the
OpenMC structured mesh *and* a matching native libMesh mesh "on the fly" from text parameters.
This makes the tally fully independent of the `[Mesh]` block - no one-to-one copy requirement.

The structured mesh is described by per-axis node coordinates. Both the OpenMC mesh and the
native libMesh mesh are built from the same coordinates, so the OpenMC tally bins line up
one-to-one with the elements of the genlib Mesh mesh (in bin ordering, i.e. x-fastest, then y,
then z, matching `openmc::StructuredMesh::get_bin_from_indices`). The tally results are read
back and stored into the generated mesh object (see `structuredMesh()`).

Two mesh types are supported via the `mesh_type` parameter:

- `regular` (default): a uniform Cartesian mesh specified by `lower_left` and either
  `upper_right` or `width`, together with the number of cells `nx`/`ny`/`nz` along each axis.
  This maps to an `openmc::RegularMesh`.
- `rectilinear`: a non-uniform Cartesian mesh specified by the grid line coordinates
  `x_grid`/`y_grid`/`z_grid`. This maps to an `openmc::RectilinearMesh`.

The number of spatial directions is given by `dimensions` (1, 2, or 3). Units should match
OpenMC's length scale (centimeters); see [OpenMCCellAverageProblem.md#scaling].

## Example Input File Syntax

!syntax parameters /Problem/Tallies/StructuredMeshTally

!syntax inputs /Problem/Tallies/StructuredMeshTally