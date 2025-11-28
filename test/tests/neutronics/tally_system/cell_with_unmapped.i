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

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Cell_1]
      type = CellTally
      score = kappa_fission
      block = '100'

      # We are intentionally excluding some blocks, so we need to disable global normalization.
      normalize_by_global_tally = false
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
  [Light_ID]
    type = PointValue
    point = '0 0 10'
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
