# NekUsrWrkBoundaryIntegral

!syntax description /Postprocessors/NekUsrWrkBoundaryIntegral

## Description

This postprocessor computes the integral of a slot in the `nrs->usrwrk` array
(sometimes called the "scratch space") over a boundary in the NekRS mesh,

\begin{equation}
p=\int_{\Gamma}-q\ d\Gamma
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh,
and $q$ is the slot in the scratch space array.

!include /boundary_specs.md

!alert warning
Because there can be *anything* in the scratch space, we do not necessarily
know what the units are. Therefore, we do not dimensionalize the results of this
postprocessor.

## Example Input Syntax

As an example, the `flux1` and `flux2` postprocessors
below will evaluate the integral of the first slot in `nrs->usrwrk`
over two different boundaries in NekRS's mesh. For conjugate heat transfer,
this particular slot represents the heat flux boundary condition sent to NekRS.

!listing test/tests/conduction/zero_flux/nek_disjoint.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekUsrWrkBoundaryIntegral

!syntax inputs /Postprocessors/NekUsrWrkBoundaryIntegral

!syntax children /Postprocessors/NekUsrWrkBoundaryIntegral
