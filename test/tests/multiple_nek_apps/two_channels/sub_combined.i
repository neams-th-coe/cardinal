[Mesh]
  [pin]
    type = FileMeshGenerator
    file = pin.e
  []
  [combine]
    type = CombinerGenerator
    inputs = pin
    positions = '-0.5 0.0 0.0
                  0.5 0.0 0.0'
  []
[]

[Variables]
  [temperature]
    initial_condition = 700.0
  []
[]

[AuxVariables]
  [avg_flux]
  []
  [source]
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
  [source]
    type = CoupledForce
    variable = temperature
    v = source
  []
[]

[BCs]
  [flux]
    type = CoupledVarNeumannBC
    variable = temperature
    boundary = 'surface'
    v = avg_flux
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = 'bottom'
    value = 500.0
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
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = avg_flux
    boundary = 'surface'
    execute_on = transfer
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
  [avg_T_pins]
    type = ElementAverageValue
    variable = temperature
  []
  [max_T_pins]
    type = NodalExtremeValue
    variable = temperature
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  hide = 'flux_integral'
[]
