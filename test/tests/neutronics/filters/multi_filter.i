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
      filters = 'Energy Polar'
    []
  []

  [Filters]
    [Energy]
      type = EnergyFilter
      # CASMO 2 group structure for testing. May result in some missed particles
      energy_boundaries = '0.0 6.25e-1 2.0e7'
    []
    [Polar]
      type = PolarAngleFilter
      num_equal_divisions = 2
    []
  []
[]

[Postprocessors]
  [Pebble_1_Flux_g1_t1]
    type = PointValue
    point = '0 0 0'
    variable = flux_g1_theta1
  []
  [Pebble_1_Flux_g2_t1]
    type = PointValue
    point = '0 0 0'
    variable = flux_g2_theta1
  []
  [Pebble_1_Flux_g1_t2]
    type = PointValue
    point = '0 0 0'
    variable = flux_g1_theta2
  []
  [Pebble_1_Flux_g2_t2]
    type = PointValue
    point = '0 0 0'
    variable = flux_g2_theta2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
