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

[Problem]
  type = OpenMCCellAverageProblem
  solid_blocks = '100'
  initial_properties = xml
  verbose = true
  solid_cell_level = 0
  normalize_by_global_tally = false
  tally_name = heat_source

  tally_type = mesh
  power = 100.0
  check_zero_tallies = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
[]

[Outputs]
  exodus = true
  hide = 'temp'
[]
