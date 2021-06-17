# NekVolumeAverage

!syntax description /Postprocessors/NekVolumeAverage

## Description

This postprocessor computes a volume average of a given field over the nekRS mesh,

\begin{equation}
p=\int_{\Omega_{nek}}fd\Omega_{nek}
\end{equation}

where $\Omega_{nek}$ is the nekRS mesh (*not* the mesh mirror constructed with
NekRSMesh). The field is specified with the `field` parameter, which may be one of
`pressure`, `temperature`, or `unity`. Setting `field = unity` is equivalent to computing
the volume.

## Example Input Syntax

As an example, the following code snippet will evaluate the volume averages of
temperature and pressure over nekRS's mesh.

!listing test/tests/postprocessors/nek_volume_average/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekVolumeAverage

!syntax inputs /Postprocessors/NekVolumeAverage

!syntax children /Postprocessors/NekVolumeAverage
