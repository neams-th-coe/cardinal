[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 96
  ny = 5
  nz = 5
  xmin = -0.5
  xmax = 0.5
  ymin = -0.025
  ymax = 0.025
  zmin = -0.025
  zmax = 0.025
[]

[Variables]
  [temperature]
    initial_condition = 500.0
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = temperature
  []
  [conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[Functions]
  [k]
    type = ParsedFunction
    expression = 'if (x<0, 1.5, 2.5)'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = 'left'
    value = 500.0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = 'right'
    value = 600.0
  []
[]

[Materials]
  [k]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 'k'
  []
[]

[Postprocessors]
  [flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'left'
  []
  [flux_right]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    diffusivity = thermal_conductivity
    boundary = 'right'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 10
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'
[]

