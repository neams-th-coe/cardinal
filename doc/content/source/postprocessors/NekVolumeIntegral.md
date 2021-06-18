# NekVolumeIntegral

!syntax description /postprocessors/NekVolumeIntegral

## Description

This postprocessor computes the integral of
a specified field over the volume of the nekRS mesh,

\begin{equation}
p=\int_{\Omega}f\ d\Omega
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the nekrs mesh (*not* the mesh mirror constructed with
NekRSMesh), and
$f$ is the specified field.
The field is specified with the `field` parameter, which may be one of
`pressure`, `temperature`, or `unity`. Setting `field = unity` is equivalent to computing
the volume.

## Example Input Syntax

As an example, the following code snippet will evaluate the volume (for `field = unity`),
volume-integrated temperature (for `field = temperature`), and volume-integrated pressure
(for `field = pressure`)
on the volume of the nekRS mesh.

!listing test/tests/postprocessors/nek_volume_integral/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekVolumeIntegral

!syntax inputs /postprocessors/NekVolumeIntegral

!syntax children /postprocessors/NekVolumeIntegral
