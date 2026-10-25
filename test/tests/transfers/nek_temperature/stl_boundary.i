[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 3

  [FieldTransfers]
    [zero]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [unity]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [temperature]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [pressure]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [vel_x]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
      field = velocity_x
    []
    [vel_y]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
      field = velocity_y
    []
    [vel_z]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
      field = velocity_z
    []
    [vel_x2]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
      field = velocity_x_squared
    []
    [vel_y2]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
      field = velocity_y_squared
    []
    [vel_z2]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
      field = velocity_z_squared
    []
    [velocity]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [scalar01]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [scalar02]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [scalar03]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'
  stl = '../small_sphere.stl'
[]

[AuxVariables]
  [moose_temp]
  []
  [moose_pressure]
  []
  [moose_vel_x]
  []
  [moose_vel_y]
  []
  [moose_vel_z]
  []
  [d_t]
  []
  [d_p]
  []
  [d_v]
  []
  [d_vx]
  []
  [d_vy]
  []
  [d_vz]
  []
  [d_vx2]
  []
  [d_vy2]
  []
  [d_vz2]
  []
  [d_s1]
  []
  [d_s2]
  []
  [d_s3]
  []
[]

[Functions]
  [moose_temp]
    type = ParsedFunction
    expression = 'exp(x)+sin(y)+x*y*z'
  []
  [moose_pressure]
    type = ParsedFunction
    expression = 'exp(x)+exp(y)+exp(z)'
  []
  [moose_vel_x]
    type = ParsedFunction
    expression = 'sin(x)+2'
  []
  [moose_vel_y]
    type = ParsedFunction
    expression = 'y+1'
  []
  [moose_vel_z]
    type = ParsedFunction
    expression = 'exp(x)*exp(y)*exp(z)'
  []
[]

[AuxKernels]
  [moose_temp]
    type = FunctionAux
    variable = moose_temp
    function = moose_temp
    block = 9999
  []
  [moose_pressure]
    type = FunctionAux
    variable = moose_pressure
    function = moose_pressure
    block = 9999
  []
  [moose_vel_x]
    type = FunctionAux
    variable = moose_vel_x
    function = moose_vel_x
    block = 9999
  []
  [moose_vel_y]
    type = FunctionAux
    variable = moose_vel_y
    function = moose_vel_y
    block = 9999
  []
  [moose_vel_z]
    type = FunctionAux
    variable = moose_vel_z
    function = moose_vel_z
    block = 9999
  []
  [d_s1]
    type = ParsedAux
    variable = d_s1
    expression = '((moose_temp + 2) - scalar01) / (moose_temp + 2)'
    coupled_variables = 'moose_temp scalar01'
    block = 9999
  []
  [d_s2]
    type = ParsedAux
    variable = d_s2
    expression = '((moose_temp + 3) - scalar02) / (moose_temp + 3)'
    coupled_variables = 'moose_temp scalar02'
    block = 9999
  []
  [d_s3]
    type = ParsedAux
    variable = d_s3
    expression = '((moose_temp + 4) - scalar03) / (moose_temp + 4)'
    coupled_variables = 'moose_temp scalar03'
    block = 9999
  []
  [d_t]
    type = ParsedAux
    variable = d_t
    expression = '(moose_temp - temperature) / moose_temp'
    coupled_variables = 'moose_temp temperature'
    block = 9999
  []
  [d_p]
    type = ParsedAux
    variable = d_p
    expression = '(moose_pressure - pressure) / moose_pressure'
    coupled_variables = 'moose_pressure pressure'
    block = 9999
  []
  [d_v]
    type = ParsedAux
    variable = d_v
    expression = '(sqrt(moose_vel_x*moose_vel_x + moose_vel_y*moose_vel_y + moose_vel_z*moose_vel_z) - velocity) / sqrt(moose_vel_x*moose_vel_x + moose_vel_y*moose_vel_y + moose_vel_z*moose_vel_z)'
    coupled_variables = 'moose_vel_x moose_vel_y moose_vel_z velocity'
    block = 9999
  []
  [d_vx]
    type = ParsedAux
    variable = d_vx
    expression = '(moose_vel_x - vel_x) / moose_vel_x'
    coupled_variables = 'moose_vel_x vel_x'
    block = 9999
  []
  [d_vy]
    type = ParsedAux
    variable = d_vy
    expression = '(moose_vel_y - vel_y) / moose_vel_y'
    coupled_variables = 'moose_vel_y vel_y'
    block = 9999
  []
  [d_vz]
    type = ParsedAux
    variable = d_vz
    expression = '(moose_vel_z - vel_z) / moose_vel_z'
    coupled_variables = 'moose_vel_z vel_z'
    block = 9999
  []
  [d_vx2]
    type = ParsedAux
    variable = d_vx2
    expression = '(moose_vel_x*moose_vel_x - vel_x2) / moose_vel_x / moose_vel_x'
    coupled_variables = 'moose_vel_x vel_x2'
    block = 9999
  []
  [d_vy2]
    type = ParsedAux
    variable = d_vy2
    expression = '(moose_vel_y*moose_vel_y - vel_y2) / moose_vel_y / moose_vel_y'
    coupled_variables = 'moose_vel_y vel_y2'
    block = 9999
  []
  [d_vz2]
    type = ParsedAux
    variable = d_vz2
    expression = '(moose_vel_z*moose_vel_z - vel_z2) / moose_vel_z / moose_vel_z'
    coupled_variables = 'moose_vel_z vel_z2'
    block = 9999
  []
[]

[Postprocessors]

  # we do not expect these to be perfectly zero, because we are interpolating the
  # function (defined in the udf file) onto the STL points
  [max_error_t]
    type = ElementExtremeValue
    variable = d_t
  []
  [min_error_t]
    type = ElementExtremeValue
    variable = d_t
    value_type = min
  []

  [max_error_s1]
    type = ElementExtremeValue
    variable = d_s1
  []
  [min_error_s1]
    type = ElementExtremeValue
    variable = d_s1
    value_type = min
  []

  [max_error_s2]
    type = ElementExtremeValue
    variable = d_s2
  []
  [min_error_s2]
    type = ElementExtremeValue
    variable = d_s2
    value_type = min
  []

  [max_error_s3]
    type = ElementExtremeValue
    variable = d_s3
  []
  [min_error_s3]
    type = ElementExtremeValue
    variable = d_s3
    value_type = min
  []

  [max_error_p]
    type = ElementExtremeValue
    variable = d_p
  []
  [min_error_p]
    type = ElementExtremeValue
    variable = d_p
    value_type = min
  []

  [max_error_v]
    type = ElementExtremeValue
    variable = d_v
  []
  [min_error_v]
    type = ElementExtremeValue
    variable = d_v
    value_type = min
  []

  [max_error_vx]
    type = ElementExtremeValue
    variable = d_vx
  []
  [min_error_vx]
    type = ElementExtremeValue
    variable = d_vx
    value_type = min
  []

  [max_error_vy]
    type = ElementExtremeValue
    variable = d_vy
  []
  [min_error_vy]
    type = ElementExtremeValue
    variable = d_vy
    value_type = min
  []

  [max_error_vz]
    type = ElementExtremeValue
    variable = d_vz
  []
  [min_error_vz]
    type = ElementExtremeValue
    variable = d_vz
    value_type = min
  []

  [max_error_vx2]
    type = ElementExtremeValue
    variable = d_vx2
  []
  [min_error_vx2]
    type = ElementExtremeValue
    variable = d_vx2
    value_type = min
  []

  [max_error_vy2]
    type = ElementExtremeValue
    variable = d_vy2
  []
  [min_error_vy2]
    type = ElementExtremeValue
    variable = d_vy2
    value_type = min
  []

  [max_error_vz2]
    type = ElementExtremeValue
    variable = d_vz2
  []
  [min_error_vz2]
    type = ElementExtremeValue
    variable = d_vz2
    value_type = min
  []
  [max_unity]
    type = ElementExtremeValue
    variable = unity
  []
  [min_unity]
    type = ElementExtremeValue
    variable = unity
    value_type = min
  []
  [max_zero]
    type = ElementExtremeValue
    variable = zero
  []
  [min_zero]
    type = ElementExtremeValue
    variable = zero
    value_type = min
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
