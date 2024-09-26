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
  # Add another pebble which doesn't exist in the OpenMC model, so it will be unmapped.
  [solid_2]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 12'
  []
  [solid_2_ids]
    type = SubdomainIDGenerator
    input = solid_2
    subdomain_id = '300'
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
    inputs = 'solid_ids solid_2_ids fluid_ids'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  density_blocks = '200'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Cell_1]
      type = CellTally
      score = kappa_fission
      blocks = '100 200'
    []
  []
[]

[Postprocessors]
  [Pebble_1_ID]
    type = PointValue
    point = '0 0 0'
    variable = cell_id
  []
  [Pebble_2_ID]
    type = PointValue
    point = '0 0 4'
    variable = cell_id
  []
  [Pebble_3_ID]
    type = PointValue
    point = '0 0 8'
    variable = cell_id
  []
  [Pebble_4_ID]
    type = PointValue
    point = '0 0 12'
    variable = cell_id
  []

  [Pebble_1_Heat]
    type = PointValue
    point = '0 0 0'
    variable = kappa_fission
  []
  [Pebble_2_Heat]
    type = PointValue
    point = '0 0 4'
    variable = kappa_fission
  []
  [Pebble_3_Heat]
    type = PointValue
    point = '0 0 8'
    variable = kappa_fission
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
