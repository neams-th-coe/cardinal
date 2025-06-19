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
    ny = 5
    nz = 5
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
  [tets]
    type = ElementsToTetrahedronsConverter
    input = name
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  scaling = 100.0
  temperature_blocks = 'left right'
  cell_level = 0
  skinner = skin
  verbose = true

  # settings to match the reference/ case
  #particles = 500000
  #inactive_batches = 200
  #batches = 1000
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 1000
  []
[]

[AuxKernels]
  [disp_x] # this executes after the [Problem]
    type = FunctionAux
    variable = disp_x
    function = disp_x
  []
[]

[Functions]
  [disp_x]
    type = ParsedFunction
    expression = 'exp((x+0.05))-1'
  []
[]

[UserObjects]
  [skin]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = 1
    temperature_min = 0
    temperature_max = 2000
    build_graveyard = true
    verbose = True
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
