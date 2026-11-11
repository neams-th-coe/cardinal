# NekUsrWrkSTLIntegral

!syntax description /Postprocessors/NekUsrWrkSTLIntegral

## Description

This postprocessor computes the integral of a slot in the `platform->app->bc->o_usrwrk` array
(sometimes called the "scratch space") over an STL file provided to the [NekRSMesh](NekRSMesh.md).

!alert warning
Because there can be *anything* in the scratch space, we do not necessarily
know what the units are. Therefore, we do not dimensionalize the results of this
postprocessor.

## Example Input Syntax

As an example, the postprocessors
below will evaluate the integrals of different slots in `platform->app->bc->o_usrwrk`
over the STL file provided in the mesh.

!listing test/tests/transfers/nek_flux/read_stl.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekUsrWrkSTLIntegral

!syntax inputs /Postprocessors/NekUsrWrkSTLIntegral
