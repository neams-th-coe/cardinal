# BulkEnergyConservation

## Description

Sets an initial condition for temperature given a heat source distribution $q$ as

\begin{equation}
\label{eq:ic1}
\int_{z_\text{min}}^{z_i}\int_{\Gamma}q\ d\Gamma dz =\dot{m}C_{p}\left\lbrack T(z)-T_\text{inlet}\right\rbrack
\end{equation}

where $z$ is the flow_direction, $\Gamma$ is the integration along directions
perpendicular to $z$, $\dot{m}$ is the fluid mass flowrate, $C_p$ is the fluid isobaric
specific heat capacity, $T(z)$ is the temperature distribution, and $T_\text{inlet}$ is
the inlet temperature. In other words, this boundary condition assumes basic bulk energy
conservation based on a specified heat source distribution with flow along one main direction.

The `BulkEnergyConservation` is not an actual object in Cardinal, but instead a convenience
wrapper around the `BulkEnergyConservationIC` that automatically sets up a number of
postprocessors necessary for computing the initial condition. The parameters that must
be set for this action include:

- `variable`: Variable(s) to apply the initial condition to
- `num_layers`: Number of layers to integrate the heat source along in [eq:ic1]
- `flow_direction`: Main flow direction, `x`, `y`, or `z`
- `mass_flowrate`: Fluid mass flowrate, $\dot{m}$
- `cp`: Fluid isobaric specific heat capacity, $C_p$
- `inlet_T`: Fluid inlet temperature, $T_\text{inlet}$

Optional parameters for this action include:

- `positive_flow_direction`: Whether the flow is in the positive `direction` (true)
  or negative `direction` (false). When set to false, this means that the heat source
  in [eq:ic1] will be integrated cumulatively from $z_i$ to $z_\text{min}$
- `direction_min`: Minimum coordinate from which to integrate the heat source, $z_\text{min}$
- `direction_max`: Maximum coordinate to which to integrate the heat source, $z_\text{max}$

If not provided, the minimum and maximum coordinates over which to integrate
the heat source are obtained from a bounding box calculation.

## Example Input Syntax

We use custom Cardinal syntax in order to simplify setup of this initial condition.
When using the action syntax, this boundary condition must be paired with
a [VolumetricHeatSource](/actions/VolumetricHeatSourceICAction.md) which sets the functional
form of the initial condition on the heat source.

As an example, below we set a sinusoidal heat source with generic form
$\sin{\left(\frac{\pi z}{H}\right)}$ for a total magnitude of 1e5 (upon volume
integration). This means that the actual initial condition is
$q_0\sin{\left(\frac{\pi z}{H}\right)}$, where $q_0$ is determined in order to
satisfy the specified total volume integral. Then, we set a corresponding
fluid temperature condition based on $q=\dot{m}C_p\Delta T$.

!listing test/tests/ics/bulk_energy_conservation_ic/action.i
  start=Cardinal
  end=Postprocessors
