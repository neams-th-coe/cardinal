[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/hex_cube.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [sphereb]
    type = FileMeshGenerator
    file = ../meshes/hex_cube.e
  []
  [solid2]
    type = SubdomainIDGenerator
    input = sphereb
    subdomain_id = '200'
  []
  [spherec]
    type = FileMeshGenerator
    file = ../meshes/hex_cube.e
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

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100 200 300'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0

  [Tallies]
    [Mesh]
      type = XDGMeshTally
      mesh_template = '../meshes/hex_cube.e'
      mesh_translations = '0 0 0
                           0 0 4
                           0 0 8'

      normalize_by_global_tally = true
      check_tally_sum = false
      output = 'unrelaxed_tally_std_dev'
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
  [pebble_1_pd]
    type = PointValue
    variable = kappa_fission
    point = '0.0 0.0 0.0'
  []
  [pebble_2_pd]
    type = PointValue
    variable = kappa_fission
    point = '0.0 0.0 4.0'
  []
  [pebble_3_pd]
    type = PointValue
    variable = kappa_fission
    point = '0.0 0.0 8.0'
  []
  [pebble_1_pd_std_dev]
    type = PointValue
    variable = kappa_fission_std_dev
    point = '0.0 0.0 0.0'
  []
  [pebble_2_pd_std_dev]
    type = PointValue
    variable = kappa_fission_std_dev
    point = '0.0 0.0 4.0'
  []
  [pebble_3_pd_std_dev]
    type = PointValue
    variable = kappa_fission_std_dev
    point = '0.0 0.0 8.0'
  []
[]

[Outputs]
  execute_on = final
  csv = true
  hide = 'temp cell_instance cell_id'
[]
