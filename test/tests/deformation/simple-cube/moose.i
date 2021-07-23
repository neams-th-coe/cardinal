[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x disp_y disp_z'
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
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
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
    value = t*x*z*(2-z)*0.05
  []
  [fn_3]
    type = ParsedFunction
    value = t*(y+1)*(y-1)*0.1
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  [../]
  [heat_source]
    type = CoupledForce
    variable = temp
    v = source_auxvar
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
    variable = disp_x
    function = fn_1
    execute_on = timestep_begin
  []
  [disp_y_calc]
    type = FunctionAux
    variable = disp_y
    function = fn_2
    execute_on = timestep_begin
  []
  [disp_z_calc]
    type = FunctionAux
    variable = disp_z
    function = fn_3
    execute_on = timestep_begin
  []
[]

[Materials]
  [./thermal]
    type = HeatConductionMaterial
    thermal_conductivity = 1.0
  [../]
[]

[BCs]
  [./bc1]
    type = FunctionDirichletBC
    variable = temp
    boundary = '1 2 3 4 5 6'
    function = temp_ansol
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

[Postprocessors]
  [./integral]
    type = ElementL2Error
    variable = temp
    function = temp_ansol
  [../]
[]

[Outputs]
  exodus = true
  hide = 'source_auxvar'
  execute_on = 'final'
[]
