[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    ny = 10
    nz = 10
    xmin = -0.2
    xmax = 0.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 2.0
  []
[]

[Variables]
  [temperature]
    initial_condition = 500.0
  []
[]

[Kernels]
  [diffusion]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = thermal_conductivity
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    value = 1000.0
    boundary = 'left'
  []
  [interface]
    type = MatchedValueBC
    variable = temperature
    v = nek_temp
    boundary = 'right'
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [nek_temp]
    initial_condition = 500.0
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    variable = flux
    diffusion_variable = temperature
    component = normal
    diffusivity = thermal_conductivity
    boundary = 'right'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = flux
    boundary = 'right'
  []
[]

[Executioner]
  type = Transient
  dt = 10.0
  nl_abs_tol = 1e-8

  steady_state_detection = true
  steady_state_tolerance = 5e-4
  steady_state_start_time = 30.0
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = timestep_end
    wait_for_first_app_init = true
  []
[]

[Transfers]
  [temperature]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    from_multi_app = nek
    variable = nek_temp
  []
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = nek
    variable = flux
    from_boundaries = 'right'
  []
  [flux_integral]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = flux_integral
    from_postprocessor = flux_integral
    to_multi_app = nek
  []
  [get_qoi]
    type = MultiAppReporterTransfer
    from_multi_app = nek
    from_reporters = 'max_temp/value'
    to_reporters = 'receive/nek_max_T'
  []
[]

[Outputs]
  print_linear_residuals = false
  hide = 'flux_integral'
[]

[Reporters]
  [receive]
    type = ConstantReporter
    real_names = 'nek_max_T'

    # This value doesnt do anything - you just need to have a dummy here
    real_values = 0.0
  []
[]
