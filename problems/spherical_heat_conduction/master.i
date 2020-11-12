[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Variables]
  [temp]
    initial_condition = 300
  []
[]

[AuxVariables]
  [nek_temp]
    initial_condition = 300
  []

  [avg_flux]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 0
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = HeatSource
    variable = temp
    function = 'openmc_heat_func'
  []
[]

[AuxKernels]
  [avg_flux]
    type = FluxAverageAux
    coupled = 'temp'
    diffusivity = thermal_conductivity
    variable = avg_flux
    boundary = '1'
  []
[]

[BCs]
  inactive = 'outside'
  [outside]
    type = DirichletBC
    variable = temp
    boundary = '1'
    value = 300
  []
  [match_nek]
    type = MatchedValueBC
    variable = temp
    boundary = '1'
    v = 'nek_temp'
  []
[]

[Functions]
  [openmc_heat_func]
    type = ParsedFunction
    value = 'openmc_heat_source'
    vars = 'openmc_heat_source'
    vals = 'openmc_heat_source'
  []
[]

[Materials]
  [hc]
    type = GenericConstantMaterial
    prop_values = '0.2' # 20 W/mK -> 0.2 W/cmK
    prop_names = 'thermal_conductivity'
  []
[]

[Executioner]
  type = Transient
  #petsc_options_iname = '-pc_type -pc_hypre_type'
  num_steps = 1000
  #petsc_options_value = 'hypre boomeramg'
  dt = 1e-4
  nl_rel_tol = 1e-5
[]

[Postprocessors]
  [total_flux]
    type = SideFluxIntegral
    diffusivity = thermal_conductivity
    variable = 'temp'
    boundary = '1'
  []
  [average_flux]
    type = SideFluxAverage
    diffusivity = thermal_conductivity
    variable = 'temp'
    boundary = '1'
  []

  [./openmc_heat_source]
    type = Receiver
  []
  [./average_temp]
    type = ElementAverageValue
    variable = temp
    execute_on = 'initial timestep_begin'
  []
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = NekApp
    input_files = 'nek.i'
    execute_on = timestep_end
  []

  [openmc]
    type = TransientMultiApp
    app_type = OpenMCApp
    input_files = 'openmc.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [nek_temp]
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    direction = from_multiapp
    multi_app = nek
    variable = nek_temp
  []
  [avg_flux]
    type = MultiAppNearestNodeTransfer
    source_variable = avg_flux
    direction = to_multiapp
    multi_app = nek
    variable = avg_flux
  []
  [total_flux_to_nek]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = total_flux
    direction = to_multiapp
    from_postprocessor = total_flux
    multi_app = nek
  []

  [./heat_source_from_openmc]
    type = MultiAppPostprocessorTransfer
    direction = from_multiapp
    multi_app = openmc
    reduction_type = average
    from_postprocessor = heat_source
    to_postprocessor = openmc_heat_source
  []
  [./average_temp_to_openmc]
    type = MultiAppPostprocessorTransfer
    direction = to_multiapp
    multi_app = openmc
    from_postprocessor = average_temp
    to_postprocessor = average_temp
  []
[]
