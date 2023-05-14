[Mesh]
  type = FileMesh
  file = solid.e
[]

[Variables]
  [temperature]
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [source]
    initial_condition = 500.0
    family = MONOMIAL
    order = CONSTANT
    block = '1 2'
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    variable = flux
    diffusion_variable = temperature
    component = normal
    diffusivity = thermal_conductivity
    boundary = 'left_pin right_pin'
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
  []
  [source]
    type = CoupledForce
    variable = temperature
    v = source
    block = '1 2'
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
  [bottom]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'bottom_brick'
    function = bottom
  []
  [pin_bottom]
    type = DirichletBC
    variable = temperature
    boundary = 'bottom_pins'
    value = 500.0
  []
[]

a = 100
b = 50
c = 450

[Functions]
  [bottom]
    type = ParsedFunction
    expression = '${a}*x*x+${b}*x+${c}'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '0.1'
  []
[]

[Postprocessors]
  [avg_T_pins]
    type = ElementAverageValue
    variable = temperature
    block = '1 2'
  []
  [avg_T_brick]
    type = ElementAverageValue
    variable = temperature
    block = '3'
  []
  [max_T_pins]
    type = NodalExtremeValue
    variable = temperature
    block = '1 2'
  []
  [max_T_brick]
    type = NodalExtremeValue
    variable = temperature
    block = '3'
  []
  [T_middle_pin1]
    type = PointValue
    variable = temperature
    point = '-0.5 0.0 0.5'
  []
  [T_middle_pin2]
    type = PointValue
    variable = temperature
    point = '0.5 0.0 0.5'
  []
  [T_middle_brick]
    type = PointValue
    variable = temperature
    point = '0.0 0.0 0.5'
  []
  [flux_right_side]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 'right'
    diffusivity = thermal_conductivity
  []
  [flux_left_side]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 'left'
    diffusivity = thermal_conductivity
  []
  [flux_bottom_brick]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 'bottom_brick'
    diffusivity = thermal_conductivity
  []
  [flux_bottom_pins]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 'bottom_pins'
    diffusivity = thermal_conductivity
  []
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
