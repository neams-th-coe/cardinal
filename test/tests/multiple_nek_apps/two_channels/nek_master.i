[Mesh]
  type = FileMesh
  file = brick.e
[]

[Variables]
  [temperature]
    initial_condition = 700.0
  []
[]

[AuxVariables]
  [flux]
    family = MONOMIAL
    order = CONSTANT
  []
  [nek_temp]
    initial_condition = 700.0
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[AuxKernels]
  [flux]
    type = DiffusionFluxAux
    variable = flux
    diffusion_variable = temperature
    component = normal
    diffusivity = thermal_conductivity
    boundary = 'left_pin right_pin'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = 'left'
    value = 500.0
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = 'right'
    value = 600.0
  []
  [bottom]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'bottom_brick'
    function = bottom
  []
  [interface]
    type = MatchedValueBC
    variable = temperature
    boundary = 'left_pin right_pin'
    v = nek_temp
  []
[]

a = 100
b = 50
c = 450

[Functions]
  [bottom]
    type = ParsedFunction
    expression = '${a}*x*x+${b}*x+${c}'
  []
[]

[Materials]
  [k]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity'
    prop_values = '0.1'
  []
[]

[MultiApps]
  [nek]
    type = TransientMultiApp
    input_files = 'nek.i'
    execute_on = 'timestep_end'
    positions = '-0.5 0.0 0.0
                  0.5 0.0 0.0'
    output_in_position = true
    wait_for_first_app_init = true
  []
[]

[Transfers]
  [temperature]
    type = MultiAppNearestNodeTransfer
    source_variable = temp
    from_multi_app = nek
    variable = nek_temp
    target_boundary = 'left_pin right_pin'
  []
  [flux]
    type = MultiAppNearestNodeTransfer
    source_variable = flux
    to_multi_app = nek
    variable = avg_flux
    target_boundary = '1'
  []
  [flux_integral]
    type = MultiAppVectorPostprocessorTransfer
    to_multi_app = nek
    postprocessor = flux_integral
    vector_postprocessor = flux
    vector_name = flux
  []
[]

[VectorPostprocessors]
  [flux]
    type = VectorOfPostprocessors
    postprocessors = 'flux_pin_left flux_pin_right'
  []
[]

[Postprocessors]
  [avg_T]
    type = ElementAverageValue
    variable = temperature
  []
  [flux_pin_left]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 'left_pin'
    diffusivity = thermal_conductivity
  []
  [flux_pin_right]
    type = SideDiffusiveFluxIntegral
    variable = temperature
    boundary = 'right_pin'
    diffusivity = thermal_conductivity
  []
  [max_T]
    type = NodalExtremeValue
    variable = temperature
  []
  [T_middle]
    type = PointValue
    variable = temperature
    point = '0.0 0.0 0.5'
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-8
  dt = 0.2
  end_time = 5
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
  execute_on = 'final'

  # just to make the final gold files smaller
  hide = 'nek_temp flux flux_pin_left flux_pin_right'
[]
