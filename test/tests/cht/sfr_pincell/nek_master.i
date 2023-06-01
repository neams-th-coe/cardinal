[Mesh]
  [circle]
    type = AnnularMeshGenerator
    nr = 10
    nt = 25
    rmin = 0
    rmax = 0.4e-2
    growth_r = -1.3
  []
  [cylinder]
    type = AdvancedExtruderGenerator
    input = circle
    heights = '0.05 0.7 0.05'
    num_layers = '5 10 5'
    direction = '0 0 1'
  []
  [transform]
    type = TransformGenerator
    input = cylinder
    transform = translate
    vector_value = '0 0 -0.4'
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
  [source]
    type = CoupledForce
    variable = temperature
    v = source
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
  #num_steps = 1
  dt = 0.2
  nl_abs_tol = 1e-8

  # In order to obtain the pseudo-steady converged case, run this until steady state is
  # detected and remove the num_steps
  steady_state_detection = true
  steady_state_tolerance = 1e-3
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek.i'
    sub_cycling = true
    execute_on = timestep_end
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
  [synchronization_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronization_to_nek
    to_multi_app = nek
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [nek_temp]
    initial_condition = 628.15
  []
  [source]
    initial_condition = 2e6
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
    prop_values = '1.5'
  []
[]

[Postprocessors]
  [synchronization_to_nek]
    type = Receiver
    default = 1.0
  []
  [flux_integral]
    type = SideIntegralVariablePostprocessor
    variable = flux
    boundary = '1'
  []
  [max_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = max
  []
  [min_temp_bison]
    type = NodalExtremeValue
    variable = temperature
    value_type = min
  []
  [max_temp_interface]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = max
  []
  [min_temp_interface]
    type = NodalExtremeValue
    variable = nek_temp
    value_type = min
  []
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = source
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'

  [screen]
    type = Console
    hide = 'synchronization_to_nek'
  []
[]
