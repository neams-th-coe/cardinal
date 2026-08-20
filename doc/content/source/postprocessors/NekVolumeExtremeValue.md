# NekVolumeExtremeValue

!syntax description /Postprocessors/NekVolumeExtremeValue

## Description

This postprocessor computes the extreme value (maximum or minimum) of
a specified field over the volume of the NekRS mesh. For `value_type = max`,
this postprocessor computes

\begin{equation}
p=\max_\Omega{v-f}
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the NekRS mesh,
$v$ is the specified field, and `f` is an optional function
provided by the `function` parameter. For `value_type = min`, this postprocessor
instead computes

\begin{equation}
p=\min_\Omega{v-f}
\end{equation}

To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](/mesh/NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS. For those familiar with the
[legacy Nek5000 fortran functions](https://www.mcs.anl.gov/~oanam/old_page/index.php/Math_subroutines_and_functions#Functions), this postprocessor is equivalent to `glamax`
and `glamin`.

!include /field_specs.md

Setting `field = unity` is equivalent to computing
1, since the extreme value of unity is 1 (`unity` is
of more use for other postprocessors).

!include /nondimensional.md

The `function`, like all
quantities in Cardinal input files, is given in dimensional form.
If the `function` depends on time, that time is either the time of
the start of the next timestep (if using the default of `execute_on = timestep_end`),
or the time of the current timestep (if using `execute_on = timestep_begin`).

## Example Input Syntax

As an example, the `max_T` postprocessor will evaluate the maximum temperature
over the volume of the NekRS mesh.

!listing tutorials/pincell_multiphysics/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekVolumeExtremeValue

!syntax inputs /Postprocessors/NekVolumeExtremeValue
