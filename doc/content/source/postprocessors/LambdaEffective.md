# LambdaEffective

## Description

`LambdaEffective` computes $\Lambda_{eff}$ given that the [OpenMC problem](OpenMCCellAverageProblem.md)
is set up to compute kinetics parameters with the method of iterative fission probabilities.

## Example Input Syntax

Shown below is an example of computing both $\beta_{eff}$ and $\Lambda_{eff}$.

!listing test/tests/neutronics/kinetics/both.i
  block=Postprocessors

!syntax parameters /Postprocessors/LambdaEffective

!syntax inputs /Postprocessors/LambdaEffective
