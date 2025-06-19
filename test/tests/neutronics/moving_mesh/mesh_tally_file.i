[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_tally_in.e
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  verbose = true
  power = 1

  # the underlying OpenMC model is not changing, so parts of the geometry become
  # uncovered as the [Mesh] moves
  check_tally_sum = false

  [Tallies]
    [heat]
      type = MeshTally
      score = 'kappa_fission'
      mesh_template = mesh_tally_in.e
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

[Executioner]
  type = Transient
[]
