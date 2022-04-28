[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x_o disp_y_o disp_z_o'
[]

[Mesh]
  type = FileMesh
  file = box.msh
  parallel_type = replicated
  order = SECOND
  displacements = 'disp_x_o disp_y_o disp_z_o'
  use_displaced_mesh = true
[]

[Variables]
  [temp]
    initial_condition = 600.0
    order = SECOND
  []
[]

[AuxVariables]
  [disp_x_o]
    order = SECOND
  []
  [disp_y_o]
    order = SECOND
  []
  [disp_z_o]
    order = SECOND
  []
[]

[Functions]
  [fn_1]
    type = ParsedFunction
    value = t*x*z*(2-z)*0.1
  []
  [fn_2]
    type = ParsedFunction
    value = t*x*z*(2-z)*0.05
  []
  [fn_3]
    type = ParsedFunction
    value = t*(y+1)*(y-1)*0.1
  []
[]

[AuxKernels]
  [disp_x_calc]
    type = FunctionAux
    variable = disp_x_o
    function = fn_1
    execute_on = timestep_begin
  []
  [disp_y_calc]
    type = FunctionAux
    variable = disp_y_o
    function = fn_2
    execute_on = timestep_begin
  []
  [disp_z_calc]
    type = FunctionAux
    variable = disp_z_o
    function = fn_3
    execute_on = timestep_begin
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 1000
  nl_max_its = 50
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-4
  l_tol = 1e-4
  l_abs_tol = 1e-4

  end_time = 3
  dt = 1
  [Quadrature]
    type = GAUSS_LOBATTO
    order = SECOND
  []
[]

[BCs]
  [bc1]
    type = DirichletBC
    variable = temp
    boundary = '1 2 3 4 5 6'
    value = 0
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
   [disp_x_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_x_o
     to_multi_app = nek
     variable = disp_x
   []
   [disp_y_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_y_o
     to_multi_app = nek
     variable = disp_y
   []
   [disp_z_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_z_o
     to_multi_app = nek
     variable = disp_z
   []
[]

[Postprocessors]
  [synchronize]
    type = Receiver
    default = 1
  []
[]
