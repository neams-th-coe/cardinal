# OpenMCMeshGenerator

!syntax description /Mesh/OpenMCMeshGenerator

## Description

This mesh generator is used to create a mirror mesh from a regular mesh loaded with OpenMC at runtime.

The OpenMC mesh used by `OpenMCMeshGenerator` is identified via the `mesh_id` parameter
and must correspond to the valid ID number of a regular mesh in the OpenMC model. The path to the
folder containing the OpenMC model can be declared using the `xml_directory` parameter.

`OpenMCMeshGenerator` retrieves the coordinates of the lower left and upper right corners of the OpenMC
regular mesh and divides them by a user-defined scaling factor (defaulting to 1)
declared via the `scaling` parameter. The number of cells in the mesh in each dimension is also directly
retrieved from OpenMC.

!alert! note title=OpenMC initialization

`OpenMCMeshGenerator` depends on an initialized state of OpenMC obtained from an `OpenMCInitAction`.

When multiple objects that depend on `OpenMCInitAction` are declared, if values for the `xml_directory`
parameter are not identical, an error is raised as OpenMC can only be initialized with one model at a
time.

!alert-end!

## Example

Here is an example showing how to set up an `OpenMCMeshGenerator` from a regular mesh declared with
an ID of 1 in the OpenMC model:

!listing test/tests/meshgenerators/openmc_mesh_generator/openmc_mesh_only.i

In this example, a scaling factor of 10 is used. This means that the dimensions retrieved from OpenMC
will be divided by 10 to construct the mesh on the Cardinal side (since OpenMC always uses units of centimeters, this means that the Cardinal mesh would have units of decimeters).

!syntax parameters /Mesh/OpenMCMeshGenerator

!syntax inputs /Mesh/OpenMCMeshGenerator
