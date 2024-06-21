[GlobalParams]
  use_displaced_mesh = true
  displacements = 'disp_x_o disp_y_o disp_z_o'
[]

[Mesh]
  type = FileMesh
  file = box.msh
  parallel_type = replicated
[]

[Variables]
  [temp]
    initial_condition = 600.0
    order = SECOND
  []
[]

[AuxVariables]
  [source_auxvar]
    order = SECOND
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
  [temp_ansol]
    order = SECOND
  []
  [difference]
    order = SECOND
  []
[]

[Functions]
  [temp_ansol]
    type = ParsedFunction
    expression = (sin(x)*sin(y)*sin(z))+5
  []
  [source_fn]
    type = ParsedFunction
    expression = 3*sin(x)*sin(y)*sin(z)
  []
  [fn_1]
    type = ParsedFunction
    expression = t*x*z*(2-z)*0.1
  []
  [fn_2]
    type = ParsedFunction
    expression = t*x*z*(2-z)*0.05
  []
  [fn_3]
    type = ParsedFunction
    expression = t*(y+1)*(y-1)*0.1
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temp
  []
  [heat_source]
    type = CoupledForce
    variable = temp
    v = source_auxvar
  []
[]

[Materials]
  [thermal]
    type = HeatConductionMaterial
    specific_heat = 1.0
    thermal_conductivity = 1.0
  []
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
  [temp_ansol]
    type = FunctionAux
    variable = temp_ansol
    function = temp_ansol
  []
  [difference]
    type = ParsedAux
    variable = difference
    expression = 'temp - temp_ansol'
    coupled_variables = 'temp temp_ansol'
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

  end_time = 5
  dt = 1
[]

[BCs]
  [bc1]
    type = FunctionDirichletBC
    variable = temp
    boundary = '1 2 3 4 5 6'
    function = temp_ansol
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
   [analytic_sol_to_nek]
     type = MultiAppGeneralFieldNearestLocationTransfer
     source_variable = temp_ansol
     to_multi_app = nek
     variable = temp_ansol
   []
   [source_to_nek]
     type = MultiAppGeneralFieldNearestLocationTransfer
     source_variable = source_auxvar
     to_multi_app = nek
     variable = heat_source
   []
   [source_integral_to_nek]
     type = MultiAppPostprocessorTransfer
     from_postprocessor = source_integral_m
     to_postprocessor = source_integral
     to_multi_app = nek
   []
   [disp_x_to_nek]
     type = MultiAppGeneralFieldNearestLocationTransfer
     source_variable = disp_x_o
     to_multi_app = nek
     variable = disp_x
   []
   [disp_y_to_nek]
     type = MultiAppGeneralFieldNearestLocationTransfer
     source_variable = disp_y_o
     to_multi_app = nek
     variable = disp_y
   []
   [disp_z_to_nek]
     type = MultiAppGeneralFieldNearestLocationTransfer
     source_variable = disp_z_o
     to_multi_app = nek
     variable = disp_z
   []
   [synchronize]
    type = MultiAppPostprocessorTransfer
    to_postprocessor = transfer_in
    from_postprocessor = synchronize
    to_multi_app = nek
  []
[]

[Postprocessors]
  [integral]
    type = ElementL2Error
    variable = temp
    function = temp_ansol
  []
  [source_integral_m]
    type = ElementIntegralVariablePostprocessor
    variable = source_auxvar
  []
  [synchronize]
    type = Receiver
    default = 1
  []
  [max_diff]
    type = NodalExtremeValue
    variable = difference
    value_type = max
  []
  [min_diff]
    type = NodalExtremeValue
    variable = difference
    value_type = min
  []
  [max_T]
    type = NodalExtremeValue
    variable = temp
    value_type = max
  []
  [min_T]
    type = NodalExtremeValue
    variable = temp
    value_type = min
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  hide = 'source_auxvar synchronize temp_ansol difference'
[]
