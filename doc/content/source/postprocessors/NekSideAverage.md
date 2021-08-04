# NekSideAverage

!syntax description /postprocessors/NekSideAverage

## Description

This postprocessor computes the average of
a specified field over a boundary in the NekRS mesh,

\begin{equation}
p=\frac{\int_{\Gamma}f\ d\Gamma}{\int_{\Gamma}\ d\Gamma}
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the NekRS mesh, and
$f$ is the specified field.

!include /boundary_specs.md

!include /field_specs.md

Setting `field = unity` is equivalent to computing
1, since the numerator will be exactly equal to the denominator (`unity` is
of more use for other postprocessors).

If running NekRS in non-dimensional form (and you have indicated the
appropriate nondimensional scales by setting `nondimensional = true`
for [NekRSProblem](/problems/NekRSProblem.md)), then the value of this postprocessor
is shown in *dimensional* units.

## Example Input Syntax

As an example, the following code snippet will evaluate the average pressure
on the NekRS outlet boundary.

!listing tutorials/fhr_reflector/cht/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekSideAverage

!syntax inputs /postprocessors/NekSideAverage

!syntax children /postprocessors/NekSideAverage
