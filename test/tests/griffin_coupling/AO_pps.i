[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4 #3
    ny = 4 #3
    elem_type = QUAD9
  []
[]

[Variables]
  active = 'u v w'

  [u]
    order = SECOND
    family = LAGRANGE
    initial_condition = 2.8
  []

  [v]
    order = SECOND
    family = LAGRANGE
    initial_condition = 5.4
  []

  [w]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [force_fn]
    type = ParsedFunction
    expression = '1-x*x+2*t'
  []

  [exact_fn]
    type = ParsedFunction
    expression = '(1-x*x)*t'
  []

  [left_bc]
    type = ParsedFunction
    expression = t
  []
[]

[Kernels]
  [time_u]
    type = TimeDerivative
    variable = u
  []

  [diff_u]
    type = Diffusion
    variable = u
  []

  [ffn_u]
    type = BodyForce
    variable = u
    function = force_fn
  []

  [time_v]
    type = TimeDerivative
    variable = v
  []

  [diff_v]
    type = Diffusion
    variable = v
  []

  [diff_w]
    type = Diffusion
    variable = w
  []
[]

[BCs]
  [all_u]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  []

  [left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = '3'
    function = left_bc
  []

  [right_v]
    type = DirichletBC
    variable = v
    boundary = '1'
    value = 0
  []

  [left_w]
    type = DirichletBC
    variable = w
    boundary = '3'
    value = 0
  []

  [right_w]
    type = DirichletBC
    variable = w
    boundary = '1'
    value = 1
  []
[]

[Postprocessors]
  [initial_u]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = initial
  []

  [initial_v]
    type = ElementIntegralVariablePostprocessor
    variable = v
    execute_on = initial
  []

  [AxialOffset_u_x]
    type = AxialOffsetPostprocessor
    variable = u
    axial_direction = x
    midpoint = 0.5
  []

  [AxialOffset_u_y]
    type = AxialOffsetPostprocessor
    variable = u
    axial_direction = y
    midpoint = 0.5
  []

  [AxialOffset_w]
    type = AxialOffsetPostprocessor
    variable = w
    axial_direction = x
    midpoint = 0.5
  []
[]

[Executioner]
  type = Transient

  dt = 0.1
  num_steps = 5
[]

[MultiApps]
  [cardinal]
    type = TransientMultiApp
    input_files = moose.i
  []
[]

[Outputs]
  file_base = out_AO_pps
  exodus = true
  csv = true
[]
