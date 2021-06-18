# NekVolumeAverage

!syntax description /postprocessors/NekVolumeAverage

## Description

This postprocessor computes a volume average of a given field over the nekRS mesh,

\begin{equation}
p=\frac{\int_{\Omega}f\ d\Omega}{\int_{\Omega}d\Omega}
\end{equation}

where $\Omega$ is the volume of the nekRS mesh (*not* the mesh mirror constructed with
NekRSMesh). The field is specified with the `field` parameter, which may be one of
`pressure`, `temperature`, or `unity`. Setting `field = unity` is equivalent to computing
unity because the numerator is exactly equal to the denominator.

## Example Input Syntax

As an example, the following code snippet will evaluate the volume averages of
temperature (for `field = temperature`) and pressure (for `field = pressure`) over nekRS's mesh.

!listing test/tests/postprocessors/nek_volume_average/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekVolumeAverage

!syntax inputs /postprocessors/NekVolumeAverage

!syntax children /postprocessors/NekVolumeAverage
