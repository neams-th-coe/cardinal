L_ref = 5.0

[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [Dimensionalize]
    L = ${L_ref}
    U = 0.2
    T = 30.0
    dT = 200.0
    rho = 1000
    Cp = 3000
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
  scaling = ${L_ref}
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Functions]
  [f]
    type = ParsedFunction
    expression = 'x+y+z'
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [max_temp]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_temp]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []

  # subtracts a function off of the temperature values; checked against the max/min
  # printed to the console by NekRS for the temperature_shift scalar which is set to
  # the nondimensional temperature
  [max_tempf]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
    function = f
    execute_on = timestep_begin
  []
  [min_tempf]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
    function = f
    execute_on = timestep_begin
  []

  [max_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = max
  []
  [min_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = min
  []

  # subtracts a constant value off of the pressure values, returning -10 in dimensional form
  [max_pf]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = max
    function = '10.0'
  []
  [min_pf]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = min
    function = '10.0'
  []

  [max_velocity]
    type = NekVolumeExtremeValue
    field = velocity
    value_type = max
  []
  [min_velocity]
    type = NekVolumeExtremeValue
    field = velocity
    value_type = min
  []
  [max_x_velocity]
    type = NekVolumeExtremeValue
    field = velocity_x
    value_type = max
  []
  [min_x_velocity]
    type = NekVolumeExtremeValue
    field = velocity_x
    value_type = min
  []
  [max_y_velocity]
    type = NekVolumeExtremeValue
    field = velocity_y
    value_type = max
  []
  [min_y_velocity]
    type = NekVolumeExtremeValue
    field = velocity_y
    value_type = min
  []
  [max_z_velocity]
    type = NekVolumeExtremeValue
    field = velocity_z
    value_type = max
  []
  [min_z_velocity]
    type = NekVolumeExtremeValue
    field = velocity_z
    value_type = min
  []
[]
