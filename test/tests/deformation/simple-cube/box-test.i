[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x_o disp_y_o disp_z_o'
[]

[Mesh]
  type = FileMesh
  file = box.msh
[]

[Variables]
  [temp]
    initial_condition = 600.0
  []
[]

[AuxVariables]
  [source_auxvar]
  []
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
  [temp_ansol]
    type = ParsedFunction
    value = (sin(x)*sin(y)*sin(z))+5
  []
  [source_fn]
    type = ParsedFunction
    value = 3*sin(x)*sin(y)*sin(z)
  []
  [fn_1]
    type = ParsedFunction
    value = t*x*z*(2-z)*0.1
  []
  [fn_2]
    type = ParsedFunction
    value = 0
  []
  [fn_3]
    type = ParsedFunction
    value = 0
  []
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
  [heat_source]
    type = CoupledForce
    variable = temp
    v = source_auxvar
  [../]
[]

[Materials]
  [./thermal]
    type = HeatConductionMaterial
    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]
[]

[AuxKernels]
  [source_auxkern]
    type = FunctionAux
    variable = source_auxvar
    function = source_fn
    execute_on = timestep_begin
  []
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
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 1000
  nl_max_its = 50
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-4
  l_tol = 1e-4
  l_abs_tol = 1e-4

  start_time = 0
  n_startup_steps = 1
  end_time = 5
  dt = 1
  dtmin = 0.0001
[]

[BCs]
  [./bc1]
    type = FunctionDirichletBC
    variable = temp
    boundary = '1 2 3 4 5 6'
    function = temp_ansol
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = NekApp
    input_files = 'nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]


[Transfers]
   [source_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = source_auxvar
     direction = to_multiapp
     multi_app = nek
     variable = heat_source
   []
   [disp_x_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_x_o
     direction = to_multiapp
     multi_app = nek
     variable = disp_x
   []
   [disp_y_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_y_o
     direction = to_multiapp
     multi_app = nek
     variable = disp_y
   []
   [disp_z_to_nek]
     type = MultiAppNearestNodeTransfer
     source_variable = disp_z_o
     direction = to_multiapp
     multi_app = nek
     variable = disp_z
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
  [./integral]
    type = ElementL2Error
    variable = temp
    function = temp_ansol
  [../]
    [synchronize]
    type = Receiver
    default = 1
  []

[]


[Outputs]
  exodus = true
  execute_on = 'final'
[]
