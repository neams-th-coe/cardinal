[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x_o disp_y_o disp_z_o'
[]

[Mesh]
  type = FileMesh
  file = pipe.exo
  parallel_type = replicated
[]

[Variables]
  [temp]
    initial_condition = 0.0
    order = FIRST
  []
[]

[Kernels]
  [dummy_kernel]
    type = Diffusion
    variable = temp
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
  [temp_ansol]
    order = SECOND
  []
[]

[Functions]
  [temp_ansol]
    type = ParsedFunction
    value = (sin(x)*sin(y)*sin(z))+5
  []
  [fn_1]
    type = ParsedFunction
    value = 0.05*0.5*t*t*sin(pi*((z+2.0)/4.0))*cos(atan2(y,x))
  []
  [fn_2]
    type = ParsedFunction
    value = 0.05*0.5*t*t*sin(pi*((z+2.0)/4.0))*sin(atan2(y,x))
  []
  [fn_3]
    type = ParsedFunction
    value = 0.0
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
  [temp_ansol]
    type = FunctionAux
    variable = temp_ansol
    function = temp_ansol
  []
[]

[Executioner]
  type = Transient
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 1000
  nl_max_its = 50
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-8
  l_abs_tol = 1e-8

  end_time = 0.05
  dt = 0.001
  [Quadrature]
    type = GAUSS_LOBATTO
    order = SECOND
  []
[]

[BCs]
  [bc1]
    type = FunctionDirichletBC
    variable = temp
    boundary = '1 2 3'
    function = temp_ansol
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
   [bdisp_x_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_x_o
     direction = to_multiapp
     multi_app = nek
     variable = disp_x
     source_boundary = 2
   []
   [bdisp_y_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_y_o
     direction = to_multiapp
     multi_app = nek
     variable = disp_y
     source_boundary = 2
   []
   [bdisp_z_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_z_o
     direction = to_multiapp
     multi_app = nek
     variable = disp_z
     source_boundary = 2
   []
   [synchronize]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    direction = to_multiapp
    from_postprocessor = synchronize
    multi_app = nek
  []
[]

[Postprocessors]
  [synchronize]
    type = Receiver
    default = 1
  []
  [icbdry_ar]
    type = AreaPostprocessor
    boundary = '2'
    execute_on = INITIAL
    use_displaced_mesh = true
  []
  [bdry_ar]
    type = AreaPostprocessor
    boundary = '2'
    use_displaced_mesh = true
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
  hide = 'temp temp_ansol disp_x_o disp_y_o disp_z_o synchronize'
[]
