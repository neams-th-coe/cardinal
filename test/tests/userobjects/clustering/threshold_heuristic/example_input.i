[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
    x_min = 0
    x_max = 10
    y_min = 0
    y_max = 10
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
    type = BooleanComboClusteringUserObject
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
