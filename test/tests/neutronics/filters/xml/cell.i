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

      check_tally_sum = false
      normalize_by_global_tally = true
    []
    [Flux]
      type = CellTally
      score = 'flux'
      block = '100 200'
      filters = 'SPH_XML'

      check_tally_sum = false
      normalize_by_global_tally = true
    []
  []

  [Filters]
    [SPH_XML]
      type = FromXMLFilter
      filter_id = 1
      bin_label = 'sph'
      allow_expansion_filters = true
    []
  []
[]

[Postprocessors]
  [Pebble_1_Flux_l0_m0]
    type = PointValue
    point = '0 0 0'
    variable = flux_sph1
  []
  [Pebble_1_Flux_l1_mn1]
    type = PointValue
    point = '0 0 0'
    variable = flux_sph2
  []
  [Pebble_1_Flux_l1_m0]
    type = PointValue
    point = '0 0 0'
    variable = flux_sph3
  []
  [Pebble_1_Flux_l1_m1]
    type = PointValue
    point = '0 0 0'
    variable = flux_sph4
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
