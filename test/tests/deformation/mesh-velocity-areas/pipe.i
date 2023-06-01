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
[]

[Functions]
  [fn_1]
    type = ParsedFunction
    expression = 0.05*0.5*t*t*sin(pi*((z+2.0)/4.0))*cos(atan2(y,x))
  []
  [fn_2]
    type = ParsedFunction
    expression = 0.05*0.5*t*t*sin(pi*((z+2.0)/4.0))*sin(atan2(y,x))
  []
  [fn_3]
    type = ParsedFunction
    expression = 0.0
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
  end_time = 0.1
  dt = 0.002

  [Quadrature]
    type = GAUSS_LOBATTO
    order = SECOND
  []
[]

[BCs]
  [bc1]
    type = DirichletBC
    variable = temp
    boundary = '1 2 3'
    value = 0.0
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'nek.i'
    execute_on = timestep_end
  []
[]

[Transfers]
   [bdisp_x_to_nek]
     type = MultiAppGeometricInterpolationTransfer
     source_variable = disp_x_o
     to_multi_app = nek
     variable = disp_x
     use_displaced_mesh = true
   []
   [bdisp_y_to_nek]
     type = MultiAppGeometricInterpolationTransfer
     source_variable = disp_y_o
     to_multi_app = nek
     variable = disp_y
     use_displaced_mesh = true
   []
   [bdisp_z_to_nek]
     type = MultiAppGeometricInterpolationTransfer
     source_variable = disp_z_o
     to_multi_app = nek
     variable = disp_z
     use_displaced_mesh = true
   []
[]

[Postprocessors]
  [area_initial]
    type = AreaPostprocessor
    boundary = '2'
    execute_on = INITIAL
  []
  [area]
    type = AreaPostprocessor
    boundary = '2'
  []
  [difference]
    type = DifferencePostprocessor
    value1 = area
    value2 = area_initial
  []
[]

[Outputs]
  csv = true
[]
