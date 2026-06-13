[Mesh]
  [circle]
    type = AnnularMeshGenerator
    nr = 8
    nt = 25
    rmin = 0
    rmax = 0.4e-2
    growth_r = -1.3
  []
  [cylinder]
    type = AdvancedExtruderGenerator
    input = circle
    heights = '0.2'
    num_layers = '10'
    direction = '0 0 1'
  []
  [transform]
    type = TransformGenerator
    input = cylinder
    transform = translate
    vector_value = '0 0 -0.1'
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
  num_steps = 10
  dt = 0.1
  nl_abs_tol = 1e-8
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek_vpp.i'
    sub_cycling = true
    execute_on = timestep_end
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
    type = MultiAppReporterTransfer
    to_reporters = 'flux_integral/value'
    from_reporters = 'flux_sidesets/flux_sidesets'
    to_multi_app = nek
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [flux_linear]
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
  [flux_linear]
    type = ProjectionAux
    variable = flux_linear
    v = flux
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '1.5'
  []
[]

[VectorPostprocessors]
  [flux_sidesets]
    type = VectorOfPostprocessors
    postprocessors = 'flux_integral'
  []
[]

[Postprocessors]
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
  file_base = 'nek_master_out'
  hide = 'flux flux_linear'
[]
