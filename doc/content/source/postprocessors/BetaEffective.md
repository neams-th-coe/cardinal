# BetaEffective

## Description

`BetaEffective` computes the effective delayed neutron fraction ($\beta_{eff}$) for point reactor kinetics calculations given that the
[OpenMC problem](OpenMCCellAverageProblem.md) is set up to compute kinetics parameters with
the method of iterative fission probabilities. $\beta_{eff}$ is the adjoint flux weighted ratio of the delayed neutron yield
(number of neutrons produced from the decay of delayed neutron precursors) to the total neutron yield
from a fission event. This is an "effective" quantity as the delayed and total yield are a weak function
of the spatial configuration of the system and the neutron spectrum. For more information on
how these parameters are computed, we refer users to the
[OpenMC documentation](https://docs.openmc.org/en/latest/usersguide/kinetics.html). `BetaEffective` allows for filtering by specific
ENDF delayed groups for use in coupled point kinetics calculations by setting `beta_type` to a number between 1 and 6.

## Example Input Syntax

Shown below is an example of computing both $\beta_{eff}$ and $\Lambda_{eff}$.

!listing test/tests/neutronics/kinetics/both.i
  block=Postprocessors

!syntax parameters /Postprocessors/BetaEffective

!syntax inputs /Postprocessors/BetaEffective
