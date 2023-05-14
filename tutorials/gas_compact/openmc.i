height = 1.60                            # height of the unit cell (m)
fluid_Cp = 5189.0                        # fluid isobaric specific heat (J/kg/K)
inlet_T = 598.0                          # inlet fluid temperature (K)
power = 30e3                             # unit cell power (W)
mdot = 0.011                             # fluid mass flowrate (kg/s)

[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid_rotated.e
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
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
    type = ParsedFunction
    expression = '${inlet_T} + z / ${height} * ${power} / ${mdot} / ${fluid_Cp}'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  output = 'unrelaxed_tally_std_dev'
  check_equal_mapped_tally_volumes = true

  power = ${power}
  scaling = 100.0
  solid_blocks = '1 2'
  tally_blocks = '2'
  tally_type = cell
  tally_name = heat_source
  solid_cell_level = 1
[]

[MultiApps]
  [solid]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'solid.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [heat_source_to_solid]
    type = MultiAppMeshFunctionTransfer
    to_multi_app = solid
    variable = power
    source_variable = heat_source
    from_postprocessors_to_be_preserved = heat_source
    to_postprocessors_to_be_preserved = power
  []
  [temperature_to_openmc]
    type = MultiAppMeshFunctionTransfer
    from_multi_app = solid
    variable = temp
    source_variable = T
  []
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    execute_on = 'transfer initial timestep_end'
  []
  [max_tally_rel_err]
    type = TallyRelativeError
    value_type = max
  []
  [max_power]
    type = ElementExtremeValue
    variable = heat_source
    value_type = max
    block = 2
  []
  [min_power]
    type = ElementExtremeValue
    variable = heat_source
    value_type = min
    block = 2
  []
[]

[UserObjects]
  [avg_power]
    type = NearestPointLayeredAverage
    variable = heat_source
    points = '0.0 0.0 0.0'
    num_layers = 30
    direction = z
    block = 2
  []
  [avg_std_dev]
    type = NearestPointLayeredAverage
    variable = heat_source_std_dev
    points = '0.0 0.0 0.0'
    num_layers = 30
    direction = z
    block = 2
  []
[]

[VectorPostprocessors]
  [avg_q]
    type = SpatialUserObjectVectorPostprocessor
    userobject = avg_power
  []
  [stdev]
    type = SpatialUserObjectVectorPostprocessor
    userobject = avg_std_dev
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  exodus = true
  csv = true
  perf_graph_live = false
[]
