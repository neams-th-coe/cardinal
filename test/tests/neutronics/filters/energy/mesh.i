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

  # Disable global normalization since we have a loosely fitting mesh tally.
  normalize_by_global_tally = false

  [Tallies]
    [Heating]
      type = CellTally
      score = 'kappa_fission'
      blocks = '100 200'
    []
    [Flux]
      type = MeshTally
      score = 'flux'
      filters = 'Energy'
    []
  []

  [Filters]
    [Energy]
      type = EnergyFilter
      # CASMO 2 group structure for testing. May result in some missed particles
      energy_boundaries = '0.0 6.25e-1 2.0e7'
    []
  []
[]

[Postprocessors]
  [Total_Flux_1]
    type = ElementIntegralVariablePostprocessor
    variable = flux_g1
  []
  [Total_Flux_2]
    type = ElementIntegralVariablePostprocessor
    variable = flux_g2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]