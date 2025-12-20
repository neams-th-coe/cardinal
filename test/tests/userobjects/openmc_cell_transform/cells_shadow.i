[Mesh]
  [block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 20
    ny = 20
    nz = 20
    xmin = -12
    xmax =  12
    ymin = -12
    ymax =   4
    zmin =   0
    zmax =  30
  []
[]

[GlobalParams]
  # These parameters are added to invoke OpenMCCellAverageProblem to redo cell mapping at each
  # time step, but the displacements will remain zero
  displacements = 'disp_x disp_y disp_z'
  use_displaced_mesh = true
[]
[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 100.0
  cell_level = 1
  reset_seed = true

  [Tallies]
    [heat]
      type = CellTally
      score = 'kappa_fission'
    []
  []
[]

[Functions]
  [shift_y_fn]
    type = ParsedFunction
    expression = '-t'
  []
[]

[Postprocessors]
  [shift_y]
    type = FunctionValuePostprocessor
    function = shift_y_fn
    execute_on = 'timestep_begin'
  []
  [k]
    type = KEigenvalue
  []
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]

[UserObjects]
  [move_bundle]
    type = OpenMCCellTransform
    transform_type = 'translation'
    vector_value = '0 shift_y 0'
    cell_ids = '2011'
    execute_on = 'timestep_begin'
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Outputs]
  exodus = true
[]
