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

[Outputs]
  csv = true
  hide = 'source_integral temp_l2_comparison_b'
  execute_on = 'final'
[]

[Functions]
  [function_to_compare]
    type = ParsedFunction
    expression = 'exp(x*x)+0.7*y+0.8*z+1'
  []
[]

[Postprocessors]
  [temp_l2]
    type = NekFunctionNorm
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
    type = NekFunctionNorm
    field = temperature
    N = 1
    function = function_to_compare
  []
  [temp_l1_comparison]
    type = NekVolumeIntegral
    field = velocity_y
  []
[]
