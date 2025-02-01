[Mesh]
  type = FileMesh
  file = ../meshes/pyramid.exo
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [temp_l2]
  []

[]

[ICs]
  [temp_l2]
    type = FunctionIC
    variable = temp_l2
    function = temp_l2
  []
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
  [pressure]
    type = ParsedFunction
    expression = 'exp(x)+exp(y)+exp(z)'
  []
  [velocity_x]
    type = ParsedFunction
    expression = 'sin(x)'
  []
  [velocity_y]
    type = ParsedFunction
    expression = 'y+1'
  []
  [velocity_z]
    type = ParsedFunction
    expression = 'exp(x*y*z)'
  []
  [function_to_compare]
    type = ParsedFunction
    expression = 'exp(x*x)+0.7*y+0.8*z+1'
  []
  [temp_l2]
    type = ParsedFunction
    expression = '(temp-function_to_compare)^2'
    symbol_names = 'temp function_to_compare'
    symbol_values = 'temp function_to_compare'
  []
[]

[Executioner]
  type = Transient
  dt = 5e-4
  num_steps = 1

  [Quadrature]
    type = GAUSS_LOBATTO
    order = FIRST
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
  hide = 'temp_l2_b'
[]

[Postprocessors]
  [temp_l2_b]
    type = ElementIntegralVariablePostprocessor
    variable = temp_l2
  []
  [temp_l2]
    type = ParsedPostprocessor
    expression = 'sqrt(temp_l2_b)'
    pp_names = 'temp_l2_b'
  []
[]
