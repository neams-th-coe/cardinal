# NekVolumeNorm

!syntax description /Postprocessors/NekVolumeNorm

## Description

This postprocessor computes the integrated L$^N$ norm of a specified
field over the volume of the NekRS mesh:

\begin{equation}
p=\left(\int_{\Omega}(f-\tilde{f})^N\ d\Omega\right)^{1/N}
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the NekRS mesh,
$f$ is the specified field, and $\tilde{f}$ is an optional shifting function
provided by `function`.

!include /field_specs.md

## Example Input Syntax

As an example, the following code snippet will evaluate the L$^2$ and L$^1$ norms
of the temperature field from NekRS relative to a function provided in the input file.

!listing test/tests/postprocessors/nek_function_norm/nek.i
  block=Postprocessors

!syntax parameters /Postprocessors/NekVolumeNorm

!syntax inputs /Postprocessors/NekVolumeNorm
