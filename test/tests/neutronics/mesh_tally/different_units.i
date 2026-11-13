[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/hex_cube_in_m.e
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

  # the [Mesh] is in units of meters, so the mesh_template must also be in units of meters
  scaling = 100.0

  [Tallies]
    [Mesh]
      type = MeshTally
      mesh_template = '../meshes/hex_cube_in_m.e'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [kappa_fission]
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
