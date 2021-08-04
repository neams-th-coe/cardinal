# NekVolumeIntegral

!syntax description /postprocessors/NekVolumeIntegral

## Description

This postprocessor computes the integral of
a specified field over the volume of the NekRS mesh,

\begin{equation}
p=\int_{\Omega}f\ d\Omega
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the nekrs mesh, and
$f$ is the specified field.
To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /field_specs.md

Setting `field = unity` is equivalent to computing the volume.

If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the following code snippet will evaluate the volume (for `field = unity`),
volume-integrated temperature (for `field = temperature`), and volume-integrated pressure
(for `field = pressure`)
on the volume of the NekRS mesh.

!listing test/tests/postprocessors/nek_volume_integral/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekVolumeIntegral

!syntax inputs /postprocessors/NekVolumeIntegral

!syntax children /postprocessors/NekVolumeIntegral
