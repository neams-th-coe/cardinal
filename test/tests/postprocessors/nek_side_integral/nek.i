[Problem]
  type = NekRSProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6 7 8'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [area_side1]
    type = NekSideIntegral
    field = unity
    boundary = '1'
  []
  [area_side2]
    type = NekSideIntegral
    field = unity
    boundary = '2'
  []
  [area_side3]
    type = NekSideIntegral
    field = unity
    boundary = '3'
  []
  [area_side4]
    type = NekSideIntegral
    field = unity
    boundary = '4'
  []
  [area_side5]
    type = NekSideIntegral
    field = unity
    boundary = '5'
  []
  [area_side6]
    type = NekSideIntegral
    field = unity
    boundary = '6'
  []
  [area_side7]
    type = NekSideIntegral
    field = unity
    boundary = '7'
  []
  [area_side8]
    type = NekSideIntegral
    field = unity
    boundary = '8'
  []
  [temp_side1]
    type = NekSideIntegral
    field = temperature
    boundary = '1'
  []
  [temp_side2]
    type = NekSideIntegral
    field = temperature
    boundary = '2'
  []
  [temp_side3]
    type = NekSideIntegral
    field = temperature
    boundary = '3'
  []
  [temp_side4]
    type = NekSideIntegral
    field = temperature
    boundary = '4'
  []
  [temp_side5]
    type = NekSideIntegral
    field = temperature
    boundary = '5'
  []
  [temp_side6]
    type = NekSideIntegral
    field = temperature
    boundary = '6'
  []
  [temp_side7]
    type = NekSideIntegral
    field = temperature
    boundary = '7'
  []
  [temp_side8]
    type = NekSideIntegral
    field = temperature
    boundary = '8'
  []
  [pressure_side1]
    type = NekSideIntegral
    field = pressure
    boundary = '1'
  []
  [pressure_side2]
    type = NekSideIntegral
    field = pressure
    boundary = '2'
  []
  [pressure_side3]
    type = NekSideIntegral
    field = pressure
    boundary = '3'
  []
  [pressure_side4]
    type = NekSideIntegral
    field = pressure
    boundary = '4'
  []
  [pressure_side5]
    type = NekSideIntegral
    field = pressure
    boundary = '5'
  []
  [pressure_side6]
    type = NekSideIntegral
    field = pressure
    boundary = '6'
  []
  [pressure_side7]
    type = NekSideIntegral
    field = pressure
    boundary = '7'
  []
  [pressure_side8]
    type = NekSideIntegral
    field = pressure
    boundary = '8'
  []
  [velocity_avg1]
    type = NekSideIntegral
    field = velocity
    boundary = '1'
  []
  [velocity_avg2]
    type = NekSideIntegral
    field = velocity
    boundary = '2'
  []
  [velocity_avg3]
    type = NekSideIntegral
    field = velocity
    boundary = '3'
  []
  [velocity_avg4]
    type = NekSideIntegral
    field = velocity
    boundary = '4'
  []
  [velocity_avg5]
    type = NekSideIntegral
    field = velocity
    boundary = '5'
  []
  [velocity_avg6]
    type = NekSideIntegral
    field = velocity
    boundary = '6'
  []
  [velocity_avg7]
    type = NekSideIntegral
    field = velocity
    boundary = '7'
  []
  [velocity_avg8]
    type = NekSideIntegral
    field = velocity
    boundary = '8'
  []
  [velocity_comp1]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '1'
  []
  [velocity_comp2]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '2'
  []
  [velocity_comp3]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '3'
  []
  [velocity_comp4]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '4'
  []
  [velocity_comp5]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '5'
  []
  [velocity_comp6]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '6'
  []
  [velocity_comp7]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '7'
  []
  [velocity_comp8]
    type = NekSideIntegral
    field = velocity_component
    velocity_direction = '0.1 -0.2 0.3'
    boundary = '8'
  []
[]
