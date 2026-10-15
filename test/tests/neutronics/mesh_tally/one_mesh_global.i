[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/hex_cube.e
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
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0

  [Tallies]
    [Mesh]
      type = MeshTally
      mesh_template = '../meshes/hex_cube.e'
      check_tally_sum = false
      normalize_by_global_tally = true
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [pebble_1_pd]
    type = PointValue
    variable = kappa_fission
    point = '0.0 0.0 0.0'
  []
[]

[Outputs]
  execute_on = final
  csv = true
  hide = 'temp  cell_instance cell_id'
[]
