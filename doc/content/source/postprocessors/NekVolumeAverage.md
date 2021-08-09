# NekVolumeAverage

!syntax description /Postprocessors/NekVolumeAverage

## Description

This postprocessor computes a volume average of a given field over the NekRS mesh,

\begin{equation}
p=\frac{\int_{\Omega}f\ d\Omega}{\int_{\Omega}d\Omega}
\end{equation}

where $\Omega$ is the volume of the NekRS mesh.
To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /field_specs.md

Setting `field = unity` is equivalent to computing
1, since the numerator will be exactly equal to the denominator (`unity` is
of more use for other postprocessors).

!include /nondimensional.md

## Example Input Syntax

As an example, the following code snippet will evaluate the volume averages of
temperature, pressure, velocity magnitude, and velocity components over NekRS's mesh.

!listing test/tests/postprocessors/nek_volume_average/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekVolumeAverage

!syntax inputs /Postprocessors/NekVolumeAverage

!syntax children /Postprocessors/NekVolumeAverage
