# CartesianGrid

!syntax description /AuxKernels/CartesianGrid

## Description

Subdivide the mesh into a 3-D Cartesian grid, and assign each bin index to an
auxiliary variable. This can be used, in place of a temperature/density distribution,
to control how the on-the-fly skinning is imposed on DAGMC models.

## Example Input Syntax

As an example, the syntax below subdivides a mesh into a 4x4x4 grid.

!listing test/tests/auxkernels/cartesian_grid/grid.i
  block=AuxKernels

!syntax parameters /AuxKernels/CartesianGrid

!syntax inputs /AuxKernels/CartesianGrid
