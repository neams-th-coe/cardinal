[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = ../../heat_source/stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  source_strength = 1e6

  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0

  initial_properties = xml

  [Tallies]
    [Cell]
      type = CellTally
      score = 'scatter'
      blocks = '100 200'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_sca]
    type = ElementIntegralVariablePostprocessor
    variable = scatter
  []
  [fluid_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 2.0'
  []
  [pebble1_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 0.0'
  []
  [pebble2_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 4.0'
  []
  [pebble3_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 8.0'
  []
  [max_err]
    type = TallyRelativeError
    tally_score = 'scatter'
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
