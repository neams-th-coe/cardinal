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

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  verbose = true
  power = 1

  [Tallies]
    [heat]
      type = MeshTally
      score = 'kappa_fission'

      # the underlying OpenMC model is not changing, so parts of the geometry become
      # uncovered as the [Mesh] moves
      check_tally_sum = false
    []
  []
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
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

[Postprocessors]
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
  [power_left]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
    block = 'left'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
