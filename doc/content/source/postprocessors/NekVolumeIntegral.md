# NekVolumeIntegral

!syntax description /Postprocessors/NekVolumeIntegral

## Description

This postprocessor computes the integral of
a specified field over the volume of the NekRS mesh,

\begin{equation}
p=\int_{\Omega}(v-f)\ d\Omega
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the nekrs mesh,
$v$ is the specified field, and $f$ is an optional function
provided by the `function` parameter.
To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /field_specs.md

Setting `field = unity` is equivalent to computing the volume.

!include /nondimensional.md

The `function`, like all
quantities in Cardinal input files, is given in dimensional form.
If the `function` depends on time, that time is either the time of
the start of the next timestep (if using the default of `execute_on = timestep_end`),
or the time of the current timestep (if using `execute_on = timestep_begin`).

## Example Input Syntax

As an example, the following code snippet will evaluate the volume and
volume-integreated temperature, pressure, and velocity magnitude
on the volume of the NekRS mesh.

!listing test/tests/postprocessors/nek_volume_integral/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekVolumeIntegral

!syntax inputs /Postprocessors/NekVolumeIntegral
