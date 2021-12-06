[Mesh]
  type = FileMesh
  file = pin.e
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
  [T_middle]
    type = PointValue
    variable = temperature
    point = '0.0 0.0 0.5'
  []
  [max_T]
    type = NodalExtremeValue
    variable = temperature
  []
  [avg_T]
    type = ElementAverageValue
    variable = temperature
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]
