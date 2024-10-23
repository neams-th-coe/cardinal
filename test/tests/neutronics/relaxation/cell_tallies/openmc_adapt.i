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

[Adaptivity]
  [Markers/uniform]
    type = UniformMarker
    mark = refine
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 100.0
  temperature_blocks = '0'
  cell_level = 1
  scaling = 100.0

  relaxation = robbins_monro

  [Tallies]
    [Cell]
      type = CellTally
      blocks = '0'
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
[]
