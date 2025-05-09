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

  # Disable global normalization since we have a loosely fitting mesh tally.
  normalize_by_global_tally = false

  [Tallies]
    [Heating]
      type = CellTally
      score = 'kappa_fission'
      block = '100'
    []
    [Flux]
      type = MeshTally
      score = 'flux'
      mesh_template = ../../meshes/sphere.e
      mesh_translations = '0 0 0
                           0 0 4
                           0 0 8'
      filters = 'Azimuthal'
    []
  []

  [Filters]
    [Azimuthal]
      type = AzimuthalAngleFilter
      num_equal_divisions = 2
    []
  []
[]

[Postprocessors]
  [Total_Flux_Omega1]
    type = ElementIntegralVariablePostprocessor
    variable = flux_omega1
  []
  [Total_Flux_Omega2]
    type = ElementIntegralVariablePostprocessor
    variable = flux_omega2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
