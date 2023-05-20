[Problem]
  kernel_coverage_check = false
  type = FEProblem
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [T]
    initial_condition = 1000.0
  []
  [p]
    initial_condition = 101325.0
  []
[]

[AuxVariables]
  [rho]
  []
[]

[AuxKernels]
  [rho]
    type = FluidDensityAux
    variable = rho
    p = p
    T = T
    fp = sodium
  []
[]

[FluidProperties]
  [sodium]
    type = SodiumLiquidFluidProperties
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
