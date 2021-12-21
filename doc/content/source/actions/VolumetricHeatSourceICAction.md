# VolumetricHeatSource

## Description

Sets an initial condition while preserving a total specified volume integral.
The `VolumetricHeatSource` is not an actual object in Cardinal, but only a convenience
wrapper around the [IntegralPreservingFunctionIC](https://mooseframework.inl.gov/source/ics/IntegralPreservingFunctionIC.html)
in MOOSE, which defines an initial condition as the combination of a function and
a total "magnitude" (integral) that should be preserved. This action applies the
initial condition:

\begin{equation}
\label{eq:ic1}
u(\vec{r}, t_0) = q_0 f(\vec{r}, t_0)
\end{equation}

where $u$ is the variable, $f$ is the function,
and $q_0$ is a scaling
factor used to preserve a total magnitude upon volume integration:

\begin{equation}
\label{eq:ic2}
q_0=\frac{Q}{\int_\Omega fd\Omega}
\end{equation}

where $Q$ is the total magnitude
and $\Omega$ is the domain of integration. The parameters that must be provided
for this action are:

- `variable`: Variable to apply the initial condition to
- `function`: Function providing the shape of the heat source
- `magnitude`: Desired integrated total magnitude of the heat source

## Example Input Syntax

We use custom Cardinal syntax in order to simplify setup of this initial condition.
As an example, below we set a sinusoidal heat source with generic form
$\sin{\left(\frac{\pi z}{H}\right)}$ for a total magnitude of 550 (upon volume
integration). This means that the actual initial condition is
$q_0\sin{\left(\frac{\pi z}{H}\right)}$, where $q_0$ is determined in order to
satisfy the specified total volume integral.

!listing test/tests/ics/volumetric_heat_source_ic/sinusoidal_z.i
  start=Cardinal
  end=Postprocessors
