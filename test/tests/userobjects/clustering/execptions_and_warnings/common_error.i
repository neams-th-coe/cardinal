[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    x_min = 0
    x_max = 10
    y_min = 0
    y_max = 10
  []
  [add_eeid]
    type = ParsedElementIDMeshGenerator
    input = generated_mesh
    extra_element_integer_names = 'value_fraction_heuristic' #names of the eeid
    values = '-1'
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
[]

[UserObjects]
  [value_fraction]
    type = ValueFractionHeuristicUserObject
    metric_variable_name = 'metric_var'
    lower_fraction = 0.2
    upper_fraction = 0.3
  []
  [boolean_combo]
    type = BooleanComboClusteringUserObject
    expression = "value_fraction"
    id_name = "value_fraction_heuristic"
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

