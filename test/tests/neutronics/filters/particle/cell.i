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
      score = 'heating'
      block = '100 200'
      filters = 'Particle'
    []
  []

  [Filters]
    [Particle]
      type = ParticleFilter
      particles = 'neutron photon electron positron'
    []
  []
[]

[Postprocessors]
  [Pebble_1_heating_neutron]
    type = PointValue
    point = '0 0 0'
    variable = heating_neutron
  []
  [Pebble_2_heating_neutron]
    type = PointValue
    point = '0 0 4'
    variable = heating_neutron
  []
  [Pebble_3_heating_neutron]
    type = PointValue
    point = '0 0 8'
    variable = heating_neutron
  []
  [Pebble_1_heating_photon]
    type = PointValue
    point = '0 0 0'
    variable = heating_photon
  []
  [Pebble_2_heating_photon]
    type = PointValue
    point = '0 0 4'
    variable = heating_photon
  []
  [Pebble_3_heating_photon]
    type = PointValue
    point = '0 0 8'
    variable = heating_photon
  []
  [Pebble_1_heating_electron]
    type = PointValue
    point = '0 0 0'
    variable = heating_electron
  []
  [Pebble_2_heating_electron]
    type = PointValue
    point = '0 0 4'
    variable = heating_electron
  []
  [Pebble_3_heating_electron]
    type = PointValue
    point = '0 0 8'
    variable = heating_electron
  []
  [Pebble_1_heating_positron]
    type = PointValue
    point = '0 0 0'
    variable = heating_positron
  []
  [Pebble_2_heating_positron]
    type = PointValue
    point = '0 0 4'
    variable = heating_positron
  []
  [Pebble_3_heating_positron]
    type = PointValue
    point = '0 0 8'
    variable = heating_positron
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
