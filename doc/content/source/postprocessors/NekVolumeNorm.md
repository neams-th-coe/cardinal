# NekVolumeNorm

!syntax description /Postprocessors/NekVolumeNorm

## Description

This postprocessor computes either a finite integrated L$^N$ norm or the
L$^\infty$ norm of a specified field over the NekRS mesh.

For finite $N \geq 1$, the postprocessor evaluates

\begin{equation}
p=\left(\int_{\Omega}\left|f-\tilde{f}\right|^N\ d\Omega\right)^{1/N},
\end{equation}

where $p$ is the value of the postprocessor,
$\Omega$ is the volume of the NekRS mesh,
$f$ is the specified field, and $\tilde{f}$ is an optional function
provided by `function`.

When `N = infinity`, the postprocessor evaluates the nodal maximum norm

\begin{equation}
p=\max_{\boldsymbol{x}_i \in \Omega}
\left|f(\boldsymbol{x}_i)-\tilde{f}(\boldsymbol{x}_i)\right|,
\end{equation}

where the maximum is taken over all NekRS GLL nodes and all MPI ranks.

The `N` parameter may be set to any real value greater than or equal to one,
or to `infinity`.

To be clear, this postprocessor is *not* evaluated on the
[NekRSMesh](NekRSMesh.md) mesh mirror, but instead on the mesh actually
used for computation in NekRS.

!include /field_specs.md

!alert note
This postprocessor does not yet support nondimensional NekRS cases.

## Example Input Syntax

The following code snippet evaluates the L$^2$, L$^1$, and L$^\infty$ norms
of the NekRS temperature field relative to a function provided in the input file.

```ini
[Postprocessors]
  [temperature_l2]
    type = NekVolumeNorm
    field = temperature
    function = exact_temperature
    N = 2
  []

  [temperature_l1]
    type = NekVolumeNorm
    field = temperature
    function = exact_temperature
    N = 1
  []

  [temperature_linf]
    type = NekVolumeNorm
    field = temperature
    function = exact_temperature
    N = infinity
  []
[]
