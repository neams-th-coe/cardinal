# ElementIDAux

!syntax description /AuxKernels/ElementIDAux

## Description

Returns the current element ID from the mesh.

## Example Input Syntax

Here is a minimal example showing how to declare `ElementIDAux`:

!listing test/tests/meshgenerators/openmc_mesh_generator/openmc_mesh_sampling.i
  block=AuxVariables AuxKernels
  remove=AuxKernels/xcoord_ak AuxKernels/ycoord_ak AuxKernels/zcoord_ak
    AuxVariables/xcoord AuxVariables/ycoord AuxVariables/zcoord

To determine the mesh elements corresponding to a given set of 3-D coordinates, 
`ElementIDAux` can be used in association with `PointValueSampler`:

!listing test/tests/meshgenerators/openmc_mesh_generator/openmc_mesh_sampling.i
  block=AuxVariables AuxKernels VectorPostprocessors
  remove=AuxKernels/xcoord_ak AuxKernels/ycoord_ak AuxKernels/zcoord_ak
    AuxVariables/xcoord AuxVariables/ycoord AuxVariables/zcoord

Integrated in a complete input file with a mesh, this will return the element ID
corresponding to each location declared in the sampler.

!syntax parameters /AuxKernels/ElementIDAux

!syntax inputs /AuxKernels/ElementIDAux
