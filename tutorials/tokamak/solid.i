[Mesh]
  [file]
    type = FileMeshGenerator
    file = tokamak.e
  []
  [add_outer_sideset]
    type = SideSetsAroundSubdomainGenerator
    input = file
    new_boundary = 'outside'
    block = 'vacuum_vessel'
  []
[]

[Variables]
  [temp]
    initial_condition = 800.0
  []
[]

[AuxVariables]
  [heat_source]
    family = MONOMIAL
    order = CONSTANT
  []
  [heat_removed_density]
    family = MONOMIAL
    order = CONSTANT
    block = 'breeder divertor'
    initial_condition = -5000
  []
[]

[AuxKernels]
  [heat_removed]
    type = FunctionAux
    function = removal
    variable = heat_removed_density
  []
[]

[Functions]
  [removal]
    type = ParsedFunction
    expression = '-to_be_removed / volume'
    symbol_names = 'to_be_removed volume'
    symbol_values = 'to_be_removed volume'
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = CoupledForce
    variable = temp
    v = heat_source
  []
  [cooling]
    type = CoupledForce
    variable = temp
    block = 'breeder divertor'
    v = heat_removed_density
  []
[]

[BCs]
  [surface]
    type = DirichletBC
    variable = temp
    boundary = 'outside'
    value = 800.0
  []
[]

[Materials]
  [k_1]
    type = GenericConstantMaterial
    prop_values = '175'
    prop_names = 'thermal_conductivity'
    block = 'plasma_facing divertor'
  []
  [k_2]
    type = GenericConstantMaterial
    prop_values = '16.3'
    prop_names = 'thermal_conductivity'
    block = 'vacuum_vessel'
  []
  [k_3]
    type = GenericConstantMaterial
    prop_values = '20.0'
    prop_names = 'thermal_conductivity'
    block = 'multiplier breeder'
  []
[]

[Executioner]
  type = Transient

  petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type -pc_hypre_type'

  steady_state_detection = true

  # you want to make this tighter for production runs
  steady_state_tolerance = 2e-2
[]

[Outputs]
  exodus = true
  csv = true
  hide = 'to_be_removed volume'
[]

[MultiApps]
  [openmc]
    type = TransientMultiApp
    input_files = 'openmc.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [heat_source_from_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = openmc
    variable = heat_source
    source_variable = heating_local
    from_postprocessors_to_be_preserved = heating
    to_postprocessors_to_be_preserved = source_integral
    search_value_conflicts = false
  []
  [temp_to_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = openmc
    variable = temp
    source_variable = temp
    search_value_conflicts = false
  []
[]

[Postprocessors]
  [source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = transfer
  []
  [removal_integral]
    type = ElementIntegralVariablePostprocessor
    variable = heat_removed_density
    execute_on = timestep_end
    block = 'breeder divertor'
  []
  [heat_loss_vessel]
    type = SideDiffusiveFluxIntegral
    variable = temp
    diffusivity = thermal_conductivity
    boundary = 'outside'
  []
  [energy_balance]
    type = ParsedPostprocessor
    expression = 'source_integral + removal_integral - heat_loss_vessel'
    pp_names = 'source_integral removal_integral heat_loss_vessel'
  []

  # determine what to set the heat removal to in order to obtain an energy balance
  [to_be_removed]
    type = ParsedPostprocessor
    expression = 'source_integral - heat_loss_vessel'
    pp_names = 'source_integral heat_loss_vessel'
  []
  [volume]
    type = VolumePostprocessor
    block = 'breeder divertor'
    execute_on = 'initial'
  []
[]
