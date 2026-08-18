[Problem]
  type = NekRSProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[AuxVariables]
  [temperature_minus_function]
  []
  [temperature]
  []
[]

[AuxKernels]
  [temperature_minus_function]
    type = FunctionAux
    variable = temperature_minus_function
    function = temperature_minus_function
  []
  [temperature]
    type = FunctionAux
    variable = temperature
    function = temperature
  []
[]

[Outputs]
  csv = true
  hide = 'temp_l2_comparison_b'
  execute_on = 'final'
[]

[Functions]
  [temperature]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
  [function_to_compare]
    type = ParsedFunction
    expression = 'exp(x*x)+0.7*y+0.8*z+1'
  []
  [temperature_minus_function]
    type = ParsedFunction
    expression = 'abs(temperature-function_to_compare)'
    symbol_names = 'temperature function_to_compare'
    symbol_values = 'temperature function_to_compare'
  []
[]

[Postprocessors]
  [temp_l2]
    type = NekVolumeNorm
    field = temperature
    function = function_to_compare
  []
  [temp_l2_comparison_b]
    type = NekVolumeIntegral
    field = velocity_x
  []
  [temp_l2_comparison]
    type = ParsedPostprocessor
    expression = 'sqrt(temp_l2_comparison_b)'
    pp_names = 'temp_l2_comparison_b'
  []

  [temp_l1]
    type = NekVolumeNorm
    field = temperature
    N = 1
    function = function_to_compare
  []
  [temp_l1_comparison]
    type = NekVolumeIntegral
    field = velocity_y
  []

  [temp_linf]
    type = NekVolumeNorm
    field = temperature
    N = infinity
    function = function_to_compare
  []
  [temp_linf_comparison]
    type = NodalExtremeValue
    variable = temperature_minus_function
  []

  [temp_without_function_linf]
    type = NekVolumeNorm
    field = temperature
    N = infinity
  []
  [temp_without_function_linf_comparison]
    type = NodalExtremeValue
    variable = temperature
  []
[]
