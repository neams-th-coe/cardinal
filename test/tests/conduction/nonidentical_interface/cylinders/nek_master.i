[Mesh]
  [circle]
    type = AnnularMeshGenerator
    rmin = 0
    rmax = 0.75
    nt = 60
    nr = 15
    growth_r = -1.2
  []
  [extrude]
    type = FancyExtruderGenerator
    input = circle
    heights = '0.5'
    num_layers = '10'
    direction = '0 0 1'
  []
  [translate]
    type = TransformGenerator
    input = extrude
    transform = translate
    vector_value = '0 0 -0.25'
  []
[]

[Variables]
  [temperature]
    initial_condition = 800.0
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
  [source]
    type = BodyForce
    variable = temperature
    function = heat_source
  []
[]

[Functions]
  [heat_source]
    type = ParsedFunction
    value = 5000.0*cos(pi*z/0.5)*exp(x)
  []
[]

[BCs]
  [interface]
    type = MatchedValueBC
    variable = temperature
    v = nek_temp
    boundary = '1'
  []
[]

[Executioner]
  type = Transient
  num_steps = 14
  dt = 0.1
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-15
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek.i'
    execute_on = 'timestep_end'
  []
[]

[Transfers]
  [temperature]
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppNearestNodeTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
    source_boundary = '1'
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
    family = MONOMIAL
    order = CONSTANT
  []
  [nek_temp]
    # Because we run nekRS _after_ MOOSE, for the very first time step, the
    # MatchedValueBC would try to set a zero-temperature condition at the interface
    # if we did not set an initial condition on nek_temp here. This is an arbitrary value
    initial_condition = 600.0
  []
  [source]
  []
[]

[ICs]
  [source]
    type = FunctionIC
    variable = source
    function = heat_source
    block = '1'
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    variable = flux
    diffusion_variable = temperature
    component = normal
    diffusivity = thermal_conductivity
    boundary = '1'
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
    boundary = '1'
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
  csv = true
[]
