[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSStandaloneProblem
  output = "ros_tensor"
  casename = 'pipe'
#  n_usrwrk_slots = 6
[]

[Functions]
  [s_11_exact]
    type = ParsedFunction
    expression = 0.0
  []
  [s_22_exact]
    type = ParsedFunction
    expression = 0.0
  []
  [s_33_exact]
    type = ParsedFunction
    expression = 0.0
  []
  [s_23_exact]
    type = ParsedFunction
    expression = 0.0
  []
  [s_12_exact]
    type = ParsedFunction
    expression = '-y*u0/R/R'
    symbol_names = 'R u0'
    symbol_values = '0.5 2.0'
  []
  [s_13_exact]
    type = ParsedFunction
    expression = '-z*u0/R/R'
    symbol_names = 'R u0'
    symbol_values = '0.5 2.0'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []

  [Quadrature]
    type = GAUSS_LOBATTO
    order = SEVENTH
  []
[]

[Postprocessors]
  [ros_s11_error]
    type = ElementL2Error
    variable = ros_s11
    function = s_11_exact
  []
  [ros_s22_error]
    type = ElementL2Error
    variable = ros_s22
    function = s_22_exact
  []
  [ros_s33_error]
    type = ElementL2Error
    variable = ros_s33
    function = s_33_exact
  []
  [ros_s12_error]
    type = ElementL2Error
    variable = ros_s12
    function = s_12_exact
  []
  [ros_s23_error]
    type = ElementL2Error
    variable = ros_s23
    function = s_23_exact
  []
  [ros_s13_error]
    type = ElementL2Error
    variable = ros_s13
    function = s_13_exact
  []
[]

[Outputs]
  exodus = true
  csv = true
#  execute_on = 'final'
[]
