# NekVolumeExtremeValue

!syntax description /postprocessors/NekVolumeExtremeValue

## Description

This postprocessor computes the extreme value (maximum or minimum) of
a specified field over the volume of the NekRS mesh. For `value_type = max`,
this postprocessor computes

\begin{equation}
p=\max_\Omega{f}
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the NekRS mesh, and
$f$ is the specified field. For `value_type = min`, this postprocessor
instead computes

\begin{equation}
p=\min_\Omega{f}
\end{equation}

To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /field_specs.md

Setting `field = unity` is equivalent to computing
1, since the extreme value of unity is 1 (`unity` is
of more use for other postprocessors).

If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the following code snippet will evaluate the maximum temperature
over the volume of the NekRS mesh.

!listing tutorials/sfr_multiphysics/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekVolumeExtremeValue

!syntax inputs /postprocessors/NekVolumeExtremeValue

!syntax children /postprocessors/NekVolumeExtremeValue
