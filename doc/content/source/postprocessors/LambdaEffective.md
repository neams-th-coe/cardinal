# LambdaEffective

## Description

`LambdaEffective` computes the neutron mean generation time ($\Lambda_{eff}$) for point reactor kinetics calculations
given that the [OpenMC problem](OpenMCCellAverageProblem.md) is set up to compute kinetics parameters with
the method of iterative fission probabilities. $\Lambda_{eff}$ is defined as the adjoint flux weighted average time between
prompt fission events. $\Lambda_{eff}$ is an "effective" quantity as the mean generation time is a strong function
of the spatial configuration of the system and the neutron spectrum. For more information on
how these parameters are computed, we refer users to the
[OpenMC documentation](https://docs.openmc.org/en/latest/usersguide/kinetics.html).

## Example Input Syntax

Shown below is an example of computing both $\beta_{eff}$ and $\Lambda_{eff}$.

!listing test/tests/neutronics/kinetics/both.i
  block=Postprocessors

!syntax parameters /Postprocessors/LambdaEffective

!syntax inputs /Postprocessors/LambdaEffective
