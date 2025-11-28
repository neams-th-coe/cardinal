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
      block = '100'

      # Disable global normalization since we have a loosely fitting mesh tally.
      normalize_by_global_tally = false
    []
    [Flux]
      type = MeshTally
      score = 'flux'
      mesh_template = ../../meshes/sphere.e
      mesh_translations = '0 0 0
                           0 0 4
                           0 0 8'
      filters = 'Energy'

      # Disable global normalization since we have a loosely fitting mesh tally.
      normalize_by_global_tally = false
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
