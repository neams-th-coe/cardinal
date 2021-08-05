# NekSideExtremeValue

!syntax description /Postprocessors/NekSideExtremeValue

## Description

This postprocessor computes the extreme value (maximum or minimum) of
a specified field over a boundary in the NekRS mesh. For `value_type = max`,
this postprocessor computes

\begin{equation}
p=\max_\Gamma{f}
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh, and
$f$ is the specified field. For `value_type = min`, this postprocessor
instead computes

\begin{equation}
p=\min_\Gamma{f}
\end{equation}

!include /boundary_specs.md

!include /field_specs.md

Setting `field = unity` is equivalent to computing
1, since the extreme value is also 1 (`unity` is
of more use for other postprocessors).

!include /nondimensional.md

## Example Input Syntax

As an example, the following code snippet will evaluate the maximum and minimum temperature,
pressure, velocity magnitude, and velocity components on the boundaries of the NekRS mesh.

!listing test/tests/postprocessors/nek_side_extrema/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekSideExtremeValue

!syntax inputs /Postprocessors/NekSideExtremeValue

!syntax children /Postprocessors/NekSideExtremeValue
