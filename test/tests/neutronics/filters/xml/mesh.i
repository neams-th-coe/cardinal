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

  check_tally_sum = false
  normalize_by_global_tally = true

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
      filters = 'SPH_XML'
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
  [Total_Flux_l0_m0]
    type = ElementIntegralVariablePostprocessor
    variable = flux_sph1
  []
  [Total_Flux_l1_mn1]
    type = ElementIntegralVariablePostprocessor
    variable = flux_sph2
  []
  [Total_Flux_l1_m0]
    type = ElementIntegralVariablePostprocessor
    variable = flux_sph3
  []
  [Total_Flux_l1_m1]
    type = ElementIntegralVariablePostprocessor
    variable = flux_sph4
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
