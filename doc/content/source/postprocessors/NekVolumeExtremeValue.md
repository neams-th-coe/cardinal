# NekVolumeExtremeValue

!syntax description /postprocessors/NekVolumeExtremeValue

## Description

This postprocessor computes the extreme value (maximum or minimum) of
a specified field over the volume of the nekRS mesh. For `value_type = max`,
this postprocessor computes

\begin{equation}
p=\max_\Omega{f}
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the nekRS mesh (*not* the mesh mirror constructed with
NekRSMesh), and
$f$ is the specified field. For `value_type = min`, this postprocessor
instead computes

\begin{equation}
p=\min_\Omega{f}
\end{equation}

The field is specified with the `field` parameter, which may be one of
`pressure`, `temperature`, or `unity`. Setting `field = unity` is equivalent to computing
unity.

If running nekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the following code snippet will evaluate the maximum and minimum temperature
(for `field = temperature`) and maximum and minimum pressure (for `field = pressure`)
over the volume of the nekRS mesh.

!listing test/tests/postprocessors/nek_volume_extrema/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekVolumeExtremeValue

!syntax inputs /postprocessors/NekVolumeExtremeValue

!syntax children /postprocessors/NekVolumeExtremeValue
