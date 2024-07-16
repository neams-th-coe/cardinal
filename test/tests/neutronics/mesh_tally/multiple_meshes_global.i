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
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []

  allow_renumbering = false
[]

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = '../meshes/sphere.e'
    mesh_translations = '0 0 0
                         0 0 4
                         0 0 8'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100 200 300'
  initial_properties = xml
  verbose = true
  cell_level = 0
  normalize_by_global_tally = true

  power = 100.0
  check_tally_sum = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [kappa_fission]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [heat_pebble1]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '100'
  []
  [heat_pebble2]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '200'
  []
  [heat_pebble3]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = '300'
  []
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp  cell_instance cell_id'
[]
