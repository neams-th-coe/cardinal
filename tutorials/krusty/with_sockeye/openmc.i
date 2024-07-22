################################################################################
## KRUSTY                                                                     ##
## Heat Pipe Microreactor Steady State                                        ##
##                                                                            ##
## Mahmoud (3/25) (openmc.i/solid.i/model03.py/krusty_scale.e)                ##
################################################################################

power = 4000           # (W)

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = krusty_scale.e
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_in]
    family = MONOMIAL
    order = CONSTANT
  []
  [material_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
  [cell_in]
    type = CellInstanceAux
    variable = cell_in
  []
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp_ic
  []
[]

[Functions]
  [temp_ic]
    type = ConstantFunction
    value = 1073
  []
[]

 
[Problem]
  type = OpenMCCellAverageProblem
  power = ${power}
  scaling = 100.0
  output = 'unrelaxed_tally_std_dev'
  temperature_blocks = 'cavity_center clamp core gap_clamp gap_ref gap_sleeve gap_vaccan MLI ref_bottom ref_top sleeve vacuum_can'
  tally_type = mesh
  mesh_template = krusty_scale.e
  tally_name = heat_source
  cell_level = 0
  # check_zero_tallies = false
  # check_equal_mapped_tally_volumes = false
  # initial_properties=xml
  inactive_batches = 100 # 5 # 100
  batches = 500 # 20 # 500
  # particles = 5000
  first_iteration_particles = 500000
  reuse_source = true
  relaxation = dufek_gudowski
  verbose= true
[]

[MultiApps]
   # active = ' '
   [solid]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'solide.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [heat_source_to_solid]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = solid
    variable = power
    source_variable = heat_source
  []
  [temperature_to_openmc]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    from_multi_app = solid
    variable = temp
    source_variable = T
  []
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
  [heat_source_integral]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = 'transfer initial timestep_end'
  []
  [tally_err]
    type = TallyRelativeError
  []
[]


[Executioner]
  type = Transient
  dt = 5000

  steady_state_detection = true
  check_aux = true
  steady_state_tolerance = 1e-06
[]


[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]

