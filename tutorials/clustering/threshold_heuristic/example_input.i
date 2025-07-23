# mesh_inputs_params
Nx = 20
Ny = 20
x_left = 0
x_right = 10
y_bottom = 0
y_up = 10
Dim = 2

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = ${Dim}
    nx = ${Nx}
    ny = ${Ny}
    x_min = ${x_left}
    x_max = ${x_right}
    y_min = ${y_bottom}
    y_max = ${y_up}
    extra_element_integers = 'boolean'
  []
[]

[AuxVariables]
  [metric_var]
    order = CONSTANT
    family = MONOMIAL
  []
  [cluster_id_aux]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [create_metric]
    type = FunctionAux
    variable = metric_var
    function = 'sqrt(x*x + y*y)'
    execute_on = 'TIMESTEP_BEGIN'
  []
  [store_element_id]
    type=ExtraElementIDAux
    extra_id_name ="boolean"
    execute_on = 'TIMESTEP_BEGIN'
    variable=cluster_id_aux
  []
[]

[UserObjects]
  [threhsold]
    type = ThresholdHeuristicsUserObject
    metric_variable_name = 'metric_var'
    threshold = 1.1
  []
  [value_diff]
    type = ValueDifferenceHeuristicUserObject
    metric_variable_name = 'metric_var'
    tolerance = 0.001
  []
  [value_range]
    type = ValueRangeHeuristicUserObject
    metric_variable_name = 'metric_var'
    tolerance_percentage = 0.2
    value = 1
  []
  [value_fraction]
    type = ValueFractionHeuristicUserObject
    metric_variable_name = 'metric_var'
    upper_fraction = 0.2
    lower_fraction = 0.1
  []
  [boolean_combo]
    type = BooleanComboHeuristicUserObejct
    expression = "( threhsold and value_diff ) or ( value_range and value_fraction )"
    id_name = "boolean"
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  solve = false
  dt = 0.1
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
