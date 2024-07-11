[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
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
    file = ../heat_source/stoplight.exo
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
    tally_score = 'heating flux'
    tally_blocks = '100 200'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'heating'
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [flux_pebble1]
    type = PointValue
    variable = flux
    point = '0 0 0'
  []
  [flux_pebble2]
    type = PointValue
    variable = flux
    point = '0 0 4'
  []
  [flux_pebble3]
    type = PointValue
    variable = flux
    point = '0 0 8'
  []
  [flux_fluid]
    type = PointValue
    variable = flux
    point = '0 0 2'
  []
[]

[Outputs]
  csv = true
[]
