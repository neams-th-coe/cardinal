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
    type = AdvancedExtruderGenerator
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
    expression = 5000.0*cos(pi*z/0.5)*exp(x)
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
    input_files = 'nek_scalar.i'
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
    from_boundaries = '1'
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

  # load in the MOOSE temperature solution, which we will compare against
  [moose_temp]
  []
  [difference]
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
  [moose_temp]
    type = SolutionAux
    variable = moose_temp
    solution = moose_temperature
    from_variable = 'temperature'
  []
  [difference]
    type = ParsedAux
    variable = difference
    expression = '(temperature - moose_temp) / moose_temp'
    coupled_variables = 'temperature moose_temp'
  []
[]

[UserObjects]
  [moose_temperature]
    type = SolutionUserObject
    mesh = nek_master_out.e
    system_variables = 'temperature'
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
  [max_rel_difference]
    type = ElementExtremeValue
    variable = difference
  []
  [min_rel_difference]
    type = ElementExtremeValue
    variable = difference
    value_type = min
  []

  # checks that the difference is sufficiently small, i.e. less than 0.1%
  [error_within_tol]
    type = ParsedPostprocessor
    expression = 'if (max_rel_difference < 1e-3, if (min_rel_difference > -1e-3, 1, 0), 0)'
    pp_names = 'max_rel_difference min_rel_difference'
  []
[]

[Outputs]
  execute_on = 'final'
  csv = true
  hide = 'flux_integral max_rel_difference min_rel_difference'
[]
