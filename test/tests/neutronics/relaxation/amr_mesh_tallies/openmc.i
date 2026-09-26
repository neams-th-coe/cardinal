[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.03'
    dy = '0.03'
    dz = '0.03'
    ix = '1'
    iy = '1'
    iz = '2'
    subdomain_id = '0'
  []
  [to_origin]
    type = TransformGenerator
    input = cmg
    transform = TRANSLATE_CENTER_ORIGIN
  []
  [up]
    type = TransformGenerator
    input = to_origin
    transform = TRANSLATE
    vector_value = '0 0 0.02'
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
  relaxation_factor = 0.3

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
