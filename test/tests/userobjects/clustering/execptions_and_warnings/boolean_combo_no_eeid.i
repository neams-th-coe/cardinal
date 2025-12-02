[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = 10
  []
[]

[AuxVariables]
  [metric_var]
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
[]

[UserObjects]
  [threshold_1]
    type = ThresholdHeuristicUserObject
    metric_variable_name = 'metric_var'
    threshold = 1.1
  []
  [threshold_2]
    type = ThresholdHeuristicUserObject
    metric_variable_name = 'metric_var'
    threshold = 1.4
    cluster_if_above_threshold = false
  []

  [boolean_combo]
    type = BooleanComboClusteringUserObject
    expression = "( threshold_1 and threshold_2 )"
    id_name = "boolean_combo"
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

