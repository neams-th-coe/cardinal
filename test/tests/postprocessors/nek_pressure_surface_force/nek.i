[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
  output = 'pressure'
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
[]

[Postprocessors]
  [pressure_x]
    type = NekPressureSurfaceForce
    boundary = '1 2 3 4 5 6'
    component = 'x'
    mesh = 'fluid'
  []
  [pressure_y]
    type = NekPressureSurfaceForce
    boundary = '1 2 3 4 5 6'
    component = 'y'
    mesh = 'fluid'
  []
  [pressure_z]
    type = NekPressureSurfaceForce
    boundary = '1 2 3 4 5 6'
    component = 'z'
    mesh = 'fluid'
  []
  [pressure_total]
    type = NekPressureSurfaceForce
    boundary = '1 2 3 4 5 6'
    component = 'total'
    mesh = 'fluid'
  []

  # These are added to compare by hand
  [pressure_x_3]
    type = SideIntegralVariablePostprocessor
    variable = P
    boundary = '3'
  []
  [pressure_x_4]
    type = SideIntegralVariablePostprocessor
    variable = P
    boundary = '4'
  []
  [pressure_x_comp]
    type = DifferencePostprocessor
    value1 = pressure_x_3
    value2 = pressure_x_4
  []
  [pressure_y_1]
    type = SideIntegralVariablePostprocessor
    variable = P
    boundary = '1'
  []
  [pressure_y_2]
    type = SideIntegralVariablePostprocessor
    variable = P
    boundary = '2'
  []
  [pressure_y_comp]
    type = DifferencePostprocessor
    value1 = pressure_y_1
    value2 = pressure_y_2
  []
  [pressure_z_6]
    type = SideIntegralVariablePostprocessor
    variable = P
    boundary = '6'
  []
  [pressure_z_5]
    type = SideIntegralVariablePostprocessor
    variable = P
    boundary = '5'
  []
  [pressure_z_comp]
    type = DifferencePostprocessor
    value1 = pressure_z_6
    value2 = pressure_z_5
  []
[]
