# OpenMCMeshGenerator

!syntax description /Mesh/OpenMCMeshGenerator

## Description

This mesh generator is used to create a mirror mesh from a regular mesh loaded with OpenMC at runtime.

!alert! note title="OpenMC initialization"

Test

!alert-end!

## Example

!listing test/tests/meshgenerators/openmc_mesh_generator/openmc_mesh_only.i
  block=Mesh

!syntax parameters /Mesh/NekMeshGenerator

!syntax inputs /Mesh/NekMeshGenerator
