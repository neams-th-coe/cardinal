# NekPointValue

!syntax description /Postprocessors/NekPointValue

## Description

This postprocessor interpolates the NekRS solution onto a given
point (using the actual NekRS solution and basis functions). For example,
if `field = temperature`, this postprocessor returns

\begin{equation}
T(x,y,z)-f(x,y,z)
\end{equation}

where $(x,y,z)$ are the coordinates of the provided point
and `f` is an optional shifting function given by `function`.
The provided point does not need to be a [!ac](GLL) point.

!include /field_specs.md

!include /nondimensional.md

## Example Input Syntax

As an example, the following code snippet will interpolate the spectral
NekRS solution onto a provided point.

!listing test/tests/postprocessors/nek_point_value/points.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekPointValue

!syntax inputs /Postprocessors/NekPointValue

!syntax children /Postprocessors/NekPointValue
