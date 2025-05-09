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

  allow_renumbering = false
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
    [Heating]
      type = CellTally
      score = 'kappa_fission'
      block = '100 200'
    []
    [Flux]
      type = CellTally
      score = 'flux'
      block = '100 200'
      filters = 'SH'
    []
  []

  [Filters]
    [SH]
      type = SphericalHarmonicsFilter
      order = 1
    []
  []
[]

[Postprocessors]
  [Pebble_1_Flux_l0_m0]
    type = PointValue
    point = '0 0 0'
    variable = flux_l0_mpos0
  []
  [Pebble_1_Flux_l1_m1]
    type = PointValue
    point = '0 0 0'
    variable = flux_l1_mpos1
  []
  [Pebble_1_Flux_l1_mn1]
    type = PointValue
    point = '0 0 0'
    variable = flux_l1_mneg1
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
