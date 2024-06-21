coupling_boundaries = '2'

[Mesh]
  type = FileMesh
  file = bison.exo
[]

[Variables]
  [temperature]
    initial_condition = 1000.0
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = temperature
  []
  [diffusion]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = thermal_conductivity
  []
[]

[Functions]
  [bottom]
    type = ParsedFunction
    value = 1500+2000*x
  []
[]

[BCs]
  [interface]
    type = MatchedValueBC
    variable = temperature
    v = nek_temp
    boundary = 'interface'
  []
  [vol2_top]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'vol2_top'
    function = bottom
  []
[]

[Executioner]
  type = Transient
  dt = 0.005
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-15
  steady_state_detection = true
  steady_state_tolerance = 1e-2
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [temperature]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
    source_boundary = ${coupling_boundaries}
  []
  [flux_integral]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
[]

[AuxVariables]
  [flux]
    order = CONSTANT
    family = MONOMIAL
  []
  [nek_temp]
    # Because we run nekRS _after_ MOOSE, for the very first time step, the
    # MatchedValueBC would try to set a zero-temperature condition at the interface
    # if we did not set an initial condition on nek_temp here. This is an arbitrary
    # value - we choose 1600 here.
    initial_condition = 1600.0
  []
  [thermal_conductivity]
    initial_condition = 2.5
  []
[]

[AuxKernels]
  [flux_var]
    type = DiffusionFluxAux
    variable = flux
    diffusion_variable = temperature
    component = normal
    diffusivity = thermal_conductivity
    boundary = ${coupling_boundaries}
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '2.5'
  []
[]

[Postprocessors]
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = flux
    boundary = ${coupling_boundaries}
  []
  [min_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
  []
  [max_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
  []
  [max_interface_T]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = max
  []
  [min_interface_T]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = min
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  print_linear_residuals = false
[]
