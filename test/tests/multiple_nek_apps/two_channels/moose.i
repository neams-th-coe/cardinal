# This is a model of two pins in a solid block, where a MOOSE heat
# conduction module is used for each pin separately. The output matches
# a different MOOSE-standalone case (moose_combined.i) that represents
# the two pins together as a single sub-app. This input was used to verify
# the Nek implementation.

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
  [sub_temp]
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
    v = sub_temp
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
  [sub]
    type = TransientMultiApp
    input_files = 'sub.i'
    execute_on = 'timestep_end'
    positions = '-0.5 0.0 0.0
                  0.5 0.0 0.0'
    output_in_position = true
  []
[]

[Transfers]
  [temperature]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = temperature
    from_multi_app = sub
    variable = sub_temp
    target_boundary = 'left_pin right_pin'
  []
  [flux]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = flux
    to_multi_app = sub
    variable = avg_flux
    from_postprocessors_to_be_preserved = 'flux_pin_left flux_pin_right'
    to_postprocessors_to_be_preserved = flux_integral
    target_boundary = 'surface'
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
  end_time = 10
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
