[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [f]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -0.05
    xmax = 0.15
    ymin = -0.05
    ymax = 0.05
    zmin = -0.05
    zmax = 0.05
    nx = 20
    ny = 1
    nz = 1
  []
  [subdomains]
    type = ParsedSubdomainIDsGenerator
    input = f
    expression = 'if (x < 0.05, 1, 2)'
  []
  [name]
    type = RenameBlockGenerator
    input = subdomains
    old_block = '1 2'
    new_block = 'left right'
  []
  [rescale]
    type = TransformGenerator
    input = name
    transform = scale
    vector_value = '100.0 100.0 100.0'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  temperature_blocks = 'left right'
  verbose = true

  # other tests cover cell IDs and instances, so we dont need to output them
  output_cell_mapping = false
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[ICs]
  [temp_l]
    type = ConstantIC
    variable = temp
    value = 1000
    block = 'left'
  []
  [temp_r]
    type = ConstantIC
    variable = temp
    value = 1200
    block = 'right'
  []
[]

[AuxKernels]
  [disp_x] # gets called for the first time after [Problem]
    type = FunctionAux
    variable = disp_x
    function = disp_x
  []
[]

[Functions]
  [disp_x]
    type = ParsedFunction
    expression = '2.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
  hide = 'temp'
[]
