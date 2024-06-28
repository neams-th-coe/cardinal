[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
[]

[AuxVariables]
  [kappa_fission]
    type = MooseVariable
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [sum_heat_sources]
    type = SumAux
    variable = kappa_fission
    values = 'kappa_fission_g1 kappa_fission_g2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0
  normalize_by_global_tally = true

  tally_type = mesh
  mesh_template = '../meshes/sphere.e'
  power = 100.0
  check_tally_sum = false

  energy_bin_boundaries = '2.e7 6.25e-1 0.0'
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [int_heat_source_g1]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission_g1
  []
  [int_heat_source_g2]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission_g2
  []
  [int_heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp  cell_instance cell_id'
[]
