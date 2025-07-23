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
    extra_element_integers = 'threshold_heuristic'
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
    extra_id_name ="threshold_heuristic"
    execute_on = 'TIMESTEP_BEGIN'
    variable=cluster_id_aux
  []
[]

[UserObjects]
  [threhsold_1]
    type = ThresholdHeuristicsUserObject
    metric_variable_name = 'metric_var'
    threshold = 1.1
  []
  [threhsold_2]
    type = ThresholdHeuristicsUserObject
    metric_variable_name = 'metric_var'
    threshold = 1.4
    cluster_if_above_threshold = false
  []

  [boolean_combo]
    type = BooleanComboHeuristicUserObejct
    expression = "( threhsold_1 and threhsold_2 )"
    id_name = "threshold_heuristic"
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
