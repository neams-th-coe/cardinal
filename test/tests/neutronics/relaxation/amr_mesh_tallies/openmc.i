[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = ../../meshes/sphere_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02'
  []
  [set_block_ids]
    type = SubdomainIDGenerator
    input = repeat
    subdomain_id = 0
  []

  allow_renumbering = false
[]

[Adaptivity]
  marker = uniform

  [Markers/uniform]
    type = UniformMarker
    mark = 'DO_NOTHING'
  []
[]

[AuxKernels]
  [temp]
    type = ConstantAux
    variable = temp
    value = 520
    execute_on = initial
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  temperature_blocks = '0'
  output_cell_mapping = 'false'
  xml_directory = '../mesh_tallies'

  cell_level = 1
  scaling = 100.0

  relaxation = 'constant'
  relaxation_factor = 0.5

  [Tallies/Mesh]
    type = MeshTally
    output = 'unrelaxed_tally'
    check_tally_sum = false
    normalize_by_global_tally = false
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
  execute_on = 'TIMESTEP_END'
  hide = 'temp uniform'
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]
