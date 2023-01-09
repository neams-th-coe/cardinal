[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere_in_m.e
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

  tally_type = mesh
  tally_name = heat_source
  power = 100.0
  check_zero_tallies = false

  # the [Mesh] is in units of meters, so the mesh_template must also be in units of meters
  scaling = 100.0
  mesh_template = '../meshes/sphere_in_m.e'

  normalize_by_global_tally = false
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
