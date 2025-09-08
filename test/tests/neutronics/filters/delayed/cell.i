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
    [DNP]
      type = CellTally
      score = 'delayed_nu_fission decay_rate'
      block = '100 200'
      filters = 'Delayed'
    []
  []

  [Filters]
    [Delayed]
      type = DelayedGroupFilter
      dnp_groups = '1 2 3 4 5 6'
    []
  []
[]

[Postprocessors]
  [Pebble_1_DFiss_1]
    type = PointValue
    point = '0 0 0'
    variable = delayed_nu_fission_d1
  []
  [Pebble_1_DFiss_2]
    type = PointValue
    point = '0 0 0'
    variable = delayed_nu_fission_d2
  []
  [Pebble_1_DFiss_3]
    type = PointValue
    point = '0 0 0'
    variable = delayed_nu_fission_d3
  []
  [Pebble_1_DFiss_4]
    type = PointValue
    point = '0 0 0'
    variable = delayed_nu_fission_d4
  []
  [Pebble_1_DFiss_5]
    type = PointValue
    point = '0 0 0'
    variable = delayed_nu_fission_d5
  []
  [Pebble_1_DFiss_6]
    type = PointValue
    point = '0 0 0'
    variable = delayed_nu_fission_d6
  []

  [Pebble_1_DRate_1]
    type = PointValue
    point = '0 0 0'
    variable = decay_rate_d1
  []
  [Pebble_1_DRate_2]
    type = PointValue
    point = '0 0 0'
    variable = decay_rate_d2
  []
  [Pebble_1_DRate_3]
    type = PointValue
    point = '0 0 0'
    variable = decay_rate_d3
  []
  [Pebble_1_DRate_4]
    type = PointValue
    point = '0 0 0'
    variable = decay_rate_d4
  []
  [Pebble_1_DRate_5]
    type = PointValue
    point = '0 0 0'
    variable = decay_rate_d5
  []
  [Pebble_1_DRate_6]
    type = PointValue
    point = '0 0 0'
    variable = decay_rate_d6
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'TIMESTEP_END'
  csv = true
[]
