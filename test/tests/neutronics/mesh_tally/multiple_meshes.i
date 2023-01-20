[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [sphereb]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid2]
    type = SubdomainIDGenerator
    input = sphereb
    subdomain_id = '200'
  []
  [spherec]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid3]
    type = SubdomainIDGenerator
    input = spherec
    subdomain_id = '300'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid1 solid2 solid3'
    positions_file = pebble_centers.txt
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  solid_blocks = '100 200 300'
  initial_properties = xml
  verbose = true
  solid_cell_level = 0
  normalize_by_global_tally = false
  mesh_translations_file = pebble_centers.txt

  tally_name = heat_source
  tally_type = mesh
  mesh_template = '../meshes/sphere.e'
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
  [heat_pebble1]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '100'
  []
  [heat_pebble2]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '200'
  []
  [heat_pebble3]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
    block = '300'
  []
[]

[Outputs]
  exodus = true
  hide = 'temp'
[]
