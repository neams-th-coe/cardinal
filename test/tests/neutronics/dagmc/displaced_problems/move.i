scale = 100.0

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 8
    ny = 4
    nz = 2
    xmin = -0.125
    ymin = -0.375
    zmin = -0.125
    xmax = 0.875
    ymax = 0.125
    zmax = 0.125
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'y>-0.125'
    block_id = 1
  []
  [convert]
    type = ElementsToTetrahedronsConverter
    input = block_1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]

[Problem]
  type=FEProblem
  solve=false
[]

[AuxVariables]
  [disp_x]
    initial_condition = 0.0
  []
  [disp_y]
    initial_condition = 0.0
  []
  [disp_z]
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [move]
    type = FunctionAux
    variable = 'disp_y'
    function = move
    execute_on = timestep_begin
  []
[]


[Functions]
  [move]
    type = ParsedFunction
    expression =  0.25
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
