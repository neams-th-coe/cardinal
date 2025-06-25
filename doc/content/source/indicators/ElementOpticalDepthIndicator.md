# ElementOpticalDepthIndicator
  id=eodi

## Description

`ElementOpticalDepthIndicator` computes an estimate of the per-element optical depth using a reaction rate and a score from a tally in the
[tallies block](AddTallyAction.md). The estimated optical depth is computed as:

\begin{equation}
\label{optical_depth}
\tau_{x} \approx \frac{R_{x}}{\Phi} h
\end{equation}

where $\tau_{x}$ is the estimated optical depth from reaction $x$, $R_{x}$ is the reaction rate (`rxn_rate`), $\Phi$ is the scalar flux, and $h$ is an estimate of
the chord length through the element traversed by neutrons/photons. There are three options for $h$ which can be set in `h_type`: the maximum vertex separation in an element (`max`), the minimum vertex separation in an element (`min`), and the cube root of the element volume (`cube_root`).

## Example Input File Syntax

The listing below shows sample usage of the `ElementOpticalDepthIndicator` to compute per-element optical depths for different `h_type`s.

!listing /tests/neutronics/indicators/od/openmc.i
  block=Indicators

!syntax parameters /Adaptivity/Indicators/ElementOpticalDepthIndicator

!syntax inputs /Adaptivity/Indicators/ElementOpticalDepthIndicator
