# NekMeshInfoPostprocessor

!syntax description /Postprocessors/NekMeshInfoPostprocessor

## Description

This postprocessor is used only for testing the [NekRSMesh](NekRSMesh.md) object.
This postprocessor can be used to extract information about the NekRS mesh.

## Example Input Syntax

As an example, the following code snippet will extract the number of elements,
number of nodes, and the node coordinates of elements in the NekRS mesh.

!listing test/tests/nek_mesh/first_order/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekMeshInfoPostprocessor

!syntax inputs /Postprocessors/NekMeshInfoPostprocessor

!syntax children /Postprocessors/NekMeshInfoPostprocessor
