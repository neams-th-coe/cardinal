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
  [s_13_exact]
    type = ParsedFunction
    expression = 0.0
  []
  [s_12_exact]
    type = ParsedFunction
    expression = '-x*u0/R/R'
    symbol_names = 'R u0'
    symbol_values = '0.01 2.0'
  []
  [s_23_exact]
    type = ParsedFunction
    expression = '-z*u0/R/R'
    symbol_names = 'R u0'
    symbol_values = '0.01 2.0'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []

  [Quadrature]
    type = GAUSS_LOBATTO
    order = FIFTH
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
  [ros_s11_vavg]
    type = NekVolumeAverage
    field = ros_s11
  []
  [ros_s22_vavg]
    type = NekVolumeAverage
    field = ros_s22
  []
  [ros_s33_vavg]
    type = NekVolumeAverage
    field = ros_s33
  []
  [ros_s12_vavg]
    type = NekVolumeAverage
    field = ros_s12
  []
  [ros_s13_vavg]
    type = NekVolumeAverage
    field = ros_s13
  []
  [ros_s23_vavg]
    type = NekVolumeAverage
    field = ros_s23
  []
[]

[Outputs]
  exodus = false
  csv = true
  execute_on = 'final'
[]
