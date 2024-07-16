[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = ../../meshes/sphere_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.06
                 0 0 0.10'
  []
  [set_block_ids]
    type = SubdomainIDGenerator
    input = repeat
    subdomain_id = 0
  []

  allow_renumbering = false
[]

[AuxKernels]
  [temp]
    type = FunctionAux
    variable = temp
    function = axial
    execute_on = initial
  []
[]

[Functions]
  [axial]
    type = ParsedFunction
    expression = '500 + z / 0.10 * 100'
  []
[]

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = ../../meshes/sphere_in_m.e
    mesh_translations = '0.0 0.0 0.02
                         0.0 0.0 0.06
                         0.0 0.0 0.10'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  temperature_blocks = '0'

  cell_level = 1
  scaling = 100.0

  output = 'unrelaxed_tally'
  relaxation = constant

  check_tally_sum = false
[]


[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
