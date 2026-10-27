[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 3

  [FieldTransfers]
    # transfer some data into the usrwrk array so that we can evaluate it on the STL
    [usrwrk00_in]
      type = NekVolumetricData
      usrwrk_slot = 0
      direction = to_nek
    []
    [usrwrk01_in]
      type = NekVolumetricData
      usrwrk_slot = 1
      direction = to_nek
    []
    [usrwrk02_in]
      type = NekVolumetricData
      usrwrk_slot = 2
      direction = to_nek
    []

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
    [usrwrk00]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [usrwrk01]
      type = NekFieldVariable
      direction = from_nek
      mesh = stl
    []
    [usrwrk02]
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
  volume = true
  stl = '../../nek_mesh/stl/small_sphere.stl'
  stl_multiplier = 0.25
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
  [d_uw0]
  []
  [d_uw1]
  []
  [d_uw2]
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
  [usrwrk00_in]
    type = ParsedFunction
    expression = 'exp(x)+1'
  []
  [usrwrk01_in]
    type = ParsedFunction
    expression = 'exp(y)+1'
  []
  [usrwrk02_in]
    type = ParsedFunction
    expression = 'exp(z)+1'
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
  [usrwrk00_in]
    type = FunctionAux
    variable = usrwrk00_in
    function = usrwrk00_in
    execute_on = timestep_begin
  []
  [usrwrk01_in]
    type = FunctionAux
    variable = usrwrk01_in
    function = usrwrk01_in
    execute_on = timestep_begin
  []
  [usrwrk02_in]
    type = FunctionAux
    variable = usrwrk02_in
    function = usrwrk02_in
    execute_on = timestep_begin
  []
  [d_uw0]
    type = ParsedAux
    variable = d_uw0
    expression = '(usrwrk00_in - usrwrk00) / usrwrk00_in'
    coupled_variables = 'usrwrk00 usrwrk00_in'
    block = 9999
  []
  [d_uw1]
    type = ParsedAux
    variable = d_uw1
    expression = '(usrwrk01_in - usrwrk01) / usrwrk01_in'
    coupled_variables = 'usrwrk01 usrwrk01_in'
    block = 9999
  []
  [d_uw2]
    type = ParsedAux
    variable = d_uw2
    expression = '(usrwrk02_in - usrwrk02) / usrwrk02_in'
    coupled_variables = 'usrwrk02 usrwrk02_in'
    block = 9999
  []
[]

[Postprocessors]

  # we do not expect these to be perfectly zero, because we are interpolating the
  # function (defined in the udf file) onto the STL points
  [max_error_t]
    type = ElementExtremeValue
    variable = d_t
    block = 9999
  []
  [min_error_t]
    type = ElementExtremeValue
    variable = d_t
    value_type = min
    block = 9999
  []

  [max_error_uw00]
    type = ElementExtremeValue
    variable = d_uw0
    block = 9999
  []
  [min_error_uw00]
    type = ElementExtremeValue
    variable = d_uw0
    value_type = min
    block = 9999
  []

  [max_error_uw01]
    type = ElementExtremeValue
    variable = d_uw1
    block = 9999
  []
  [min_error_uw01]
    type = ElementExtremeValue
    variable = d_uw1
    value_type = min
    block = 9999
  []

  [max_error_uw02]
    type = ElementExtremeValue
    variable = d_uw2
    block = 9999
  []
  [min_error_uw02]
    type = ElementExtremeValue
    variable = d_uw2
    value_type = min
    block = 9999
  []

  [max_error_s1]
    type = ElementExtremeValue
    variable = d_s1
    block = 9999
  []
  [min_error_s1]
    type = ElementExtremeValue
    variable = d_s1
    value_type = min
    block = 9999
  []

  [max_error_s2]
    type = ElementExtremeValue
    variable = d_s2
    block = 9999
  []
  [min_error_s2]
    type = ElementExtremeValue
    variable = d_s2
    value_type = min
    block = 9999
  []

  [max_error_s3]
    type = ElementExtremeValue
    variable = d_s3
    block = 9999
  []
  [min_error_s3]
    type = ElementExtremeValue
    variable = d_s3
    value_type = min
    block = 9999
  []

  [max_error_p]
    type = ElementExtremeValue
    variable = d_p
    block = 9999
  []
  [min_error_p]
    type = ElementExtremeValue
    variable = d_p
    value_type = min
    block = 9999
  []

  [max_error_v]
    type = ElementExtremeValue
    variable = d_v
    block = 9999
  []
  [min_error_v]
    type = ElementExtremeValue
    variable = d_v
    value_type = min
    block = 9999
  []

  [max_error_vx]
    type = ElementExtremeValue
    variable = d_vx
    block = 9999
  []
  [min_error_vx]
    type = ElementExtremeValue
    variable = d_vx
    value_type = min
    block = 9999
  []

  [max_error_vy]
    type = ElementExtremeValue
    variable = d_vy
    block = 9999
  []
  [min_error_vy]
    type = ElementExtremeValue
    variable = d_vy
    value_type = min
    block = 9999
  []

  [max_error_vz]
    type = ElementExtremeValue
    variable = d_vz
    block = 9999
  []
  [min_error_vz]
    type = ElementExtremeValue
    variable = d_vz
    value_type = min
    block = 9999
  []

  [max_error_vx2]
    type = ElementExtremeValue
    variable = d_vx2
    block = 9999
  []
  [min_error_vx2]
    type = ElementExtremeValue
    variable = d_vx2
    value_type = min
    block = 9999
  []

  [max_error_vy2]
    type = ElementExtremeValue
    variable = d_vy2
    block = 9999
  []
  [min_error_vy2]
    type = ElementExtremeValue
    variable = d_vy2
    value_type = min
    block = 9999
  []

  [max_error_vz2]
    type = ElementExtremeValue
    variable = d_vz2
    block = 9999
  []
  [min_error_vz2]
    type = ElementExtremeValue
    variable = d_vz2
    value_type = min
    block = 9999
  []
  [max_unity]
    type = ElementExtremeValue
    variable = unity
    block = 9999
  []
  [min_unity]
    type = ElementExtremeValue
    variable = unity
    value_type = min
    block = 9999
  []
  [max_zero]
    type = ElementExtremeValue
    variable = zero
    block = 9999
  []
  [min_zero]
    type = ElementExtremeValue
    variable = zero
    value_type = min
    block = 9999
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
