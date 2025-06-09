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
      filters = 'Azimuthal'
    []
  []

  [Filters]
    [Azimuthal]
      type = AzimuthalAngleFilter
    []
  []
[]

[Postprocessors]
  [Pebble_1_Flux_Omega1]
    type = PointValue
    point = '0 0 0'
    variable = flux_omega1
  []
  [Pebble_2_Flux_Omega2]
    type = PointValue
    point = '0 0 4'
    variable = flux_omega2
  []
  [Pebble_3_Flux_Omega1]
    type = PointValue
    point = '0 0 8'
    variable = flux_omega1
  []
  [Pebble_1_Flux_Omega2]
    type = PointValue
    point = '0 0 0'
    variable = flux_omega2
  []
  [Pebble_2_Flux_Omega1]
    type = PointValue
    point = '0 0 4'
    variable = flux_omega1
  []
  [Pebble_3_Flux_Omega2]
    type = PointValue
    point = '0 0 8'
    variable = flux_omega2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
