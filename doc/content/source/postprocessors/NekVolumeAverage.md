# NekVolumeAverage

!syntax description /Postprocessors/NekVolumeAverage

## Description

This postprocessor computes a volume average of a given field over the NekRS mesh,

\begin{equation}
p=\frac{\int_{\Omega}(v-f)\ d\Omega}{\int_{\Omega}d\Omega}
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the NekRS mesh,
$v$ is the specified field, and $f$ is an optional function
To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /field_specs.md

Setting `field = unity` is equivalent to computing
1, since the numerator will be exactly equal to the denominator (`unity` is
of more use for other postprocessors).

!include /nondimensional.md

The `function`, like all
quantities in Cardinal input files, is given in dimensional form.
If the `function` depends on time, that time is either the time of
the start of the next timestep (if using the default of `execute_on = timestep_end`),
or the time of the current timestep (if using `execute_on = timestep_begin`).

## Example Input Syntax

As an example, the following code snippet will evaluate the volume averages of
temperature, pressure, velocity magnitude, and velocity components over NekRS's mesh.

!listing test/tests/postprocessors/nek_volume_average/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekVolumeAverage

!syntax inputs /Postprocessors/NekVolumeAverage
