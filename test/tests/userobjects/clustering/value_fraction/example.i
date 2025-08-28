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
    extra_element_integers = 'extreme_value'
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
    extra_id_name ="extreme_value"
    execute_on = 'TIMESTEP_BEGIN'
    variable=cluster_id_aux
  []
[]

[UserObjects]
  [value_fraction]
    type = ValueFractionHeuristicUserObject
    metric_variable_name = 'metric_var'
    upper_fraction = 0.2
    lower_fraction = 0.1
  []
  [boolean_combo]
    type = BooleanComboClusteringUserObject
    expression = "value_fraction"
    id_name = "extreme_value"
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