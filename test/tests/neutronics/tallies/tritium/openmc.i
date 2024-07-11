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

[Tallies]
  [Cell]
    type = CellTally
    tally_score = 'H3_production'
    tally_blocks = '100 200'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  source_strength = 1e6
  verbose = true

  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0

  initial_properties = xml
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_H3]
    type = ElementIntegralVariablePostprocessor
    variable = H3_production
  []
  [fluid_H3]
    type = PointValue
    variable = H3_production
    point = '0.0 0.0 2.0'
  []
  [pebble1_H3]
    type = PointValue
    variable = H3_production
    point = '0.0 0.0 0.0'
  []
  [pebble2_H3]
    type = PointValue
    variable = H3_production
    point = '0.0 0.0 4.0'
  []
  [pebble3_H3]
    type = PointValue
    variable = H3_production
    point = '0.0 0.0 8.0'
  []
  [vol_fluid]
    type = VolumePostprocessor
    block = '200'
  []
  [vol_solid]
    type = VolumePostprocessor
    block = '100'
  []
  [max_err]
    type = TallyRelativeError
    tally_score = 'H3_production'
  []
[]

[Outputs]
  execute_on = final
  csv = true
  exodus = true
[]
