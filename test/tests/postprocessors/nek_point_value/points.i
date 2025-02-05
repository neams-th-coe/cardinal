[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
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

[Functions]
  [f]
    type = ParsedFunction
    expression = 'x+y+z+t*10'
  []
  [s]
    type = ParsedFunction
    expression = 'sin(x)+sin(y)+z+10*t'
  []
[]

[Postprocessors]
  [temp]
    type = NekPointValue
    field = temperature
    point = '0.25 0.3 0.27'
  []
  [vx]
    type = NekPointValue
    field = velocity_x
    point = '0.25 0.3 0.27'
  []
  [vy]
    type = NekPointValue
    field = velocity_y
    point = '0.25 0.3 0.27'
  []
  [vz]
    type = NekPointValue
    field = velocity_z
    point = '0.25 0.3 0.27'
  []
  [vx2]
    type = NekPointValue
    field = velocity_x_squared
    point = '0.25 0.3 0.27'
  []
  [vy2]
    type = NekPointValue
    field = velocity_y_squared
    point = '0.25 0.3 0.27'
  []
  [vz2]
    type = NekPointValue
    field = velocity_z_squared
    point = '0.25 0.3 0.27'
  []
  [vel]
    type = NekPointValue
    field = velocity
    point = '0.25 0.3 0.27'
  []
  [p]
    type = NekPointValue
    field = pressure
    point = '0.25 0.3 0.27'
  []
  [scalar01]
    type = NekPointValue
    field = scalar01
    point = '0.25 0.3 0.27'
  []
  [scalar02]
    type = NekPointValue
    field = scalar02
    point = '0.25 0.3 0.27'
  []
  [scalar03]
    type = NekPointValue
    field = scalar03
    point = '0.25 0.3 0.27'
  []
  [vc]
    type = NekPointValue
    field = velocity_component
    velocity_direction = '1 1 1'
    point = '0.25 0.3 0.27'
  []
  [function]
    type = NekPointValue
    field = function
    function = f
    point = '0.25 0.3 0.27'
  []
  [vx_shift]
    type = NekPointValue
    field = velocity_x
    shift_field_by = s
    point = '0.25 0.3 0.27'
  []
  [T_shift]
    type = NekPointValue
    field = temperature
    shift_field_by = s
    point = '0.25 0.3 0.27'
  []
[]

[Outputs]
  csv = true
[]
