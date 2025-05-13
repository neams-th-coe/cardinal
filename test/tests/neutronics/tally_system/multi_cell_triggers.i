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

  max_batches = 200

  [Tallies]
    [Cell_1]
      type = CellTally
      score = kappa_fission
      block = '100 200'
      trigger = rel_err
      trigger_threshold = 1e-2
    []
    [Cell_2]
      type = CellTally
      score = flux
      block = '100 200'
      trigger = rel_err
      trigger_threshold = 1e-2
    []
  []
[]

[Postprocessors]
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
  [Pebble_1_Flux]
    type = PointValue
    point = '0 0 0'
    variable = flux
  []
  [Pebble_2_Flux]
    type = PointValue
    point = '0 0 4'
    variable = flux
  []
  [Pebble_3_Flux]
    type = PointValue
    point = '0 0 8'
    variable = flux
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
