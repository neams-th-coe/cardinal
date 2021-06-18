# NekSideAverage

!syntax description /postprocessors/NekSideAverage

## Description

This postprocessor computes the average of
a specified field over a boundary in the nekRS mesh,

\begin{equation}
p=\frac{\int_{\Gamma}f\ d\Gamma}{\int_{\Gamma}\ d\Gamma}
\end{equation}

where $p$ is the value of the postprocessor,
$\Gamma$ is the boundary of the nekRS mesh (*not* the mesh mirror constructed with
NekRSMesh), and
$f$ is the specified field.
The boundaries over which to integrate in
the nekRS mesh are specified with the `boundary` parameter; these boundaries
are the sidesets in nekRS's mesh (i.e. the `.re2` file). The field is specified with the `field` parameter, which may be one of
`pressure`, `temperature`, or `unity`. Setting `field = unity` is equivalent to computing
unity, since the numerator will be exactly equal to the denominator.

## Example Input Syntax

As an example, the following code snippet will evaluate the average temperature (for `field = temperature`)
and pressure (for `field = pressure`)
on the boundaries of the nekRS mesh.

!listing test/tests/postprocessors/nek_side_average/nek.i
  block=Postprocessors

!syntax parameters /postprocessors/NekSideAverage

!syntax inputs /postprocessors/NekSideAverage

!syntax children /postprocessors/NekSideAverage
