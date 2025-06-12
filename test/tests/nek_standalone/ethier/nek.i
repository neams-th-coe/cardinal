[Mesh]
  type = NekRSMesh
  volume = true
  order = SECOND
[]

[Problem]
  type = NekRSProblem
  casename = 'ethier'

  [FieldTransfers]
    [temp]
      type = NekFieldVariable
      direction = from_nek
      field = temperature
    []
    [P]
      type = NekFieldVariable
      direction = from_nek
      field = pressure
    []
    [vel_x]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_x
    []
    [vel_y]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_y
    []
    [vel_z]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_z
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [heat_flux]
    type = NekHeatFluxIntegral
    boundary = '1'
  []

  # These postprocessors are just used to verify correct extraction of the
  # NekRS solution, because we can directly compare against the values printed
  # to stdout by NekRS
  [max_Vx]
    type = NodalExtremeValue
    variable = vel_x
    value_type = max
  []
  [min_Vx]
    type = NodalExtremeValue
    variable = vel_x
    value_type = min
  []
  [max_Vy]
    type = NodalExtremeValue
    variable = vel_y
    value_type = max
  []
  [min_Vy]
    type = NodalExtremeValue
    variable = vel_y
    value_type = min
  []
  [max_Vz]
    type = NodalExtremeValue
    variable = vel_z
    value_type = max
  []
  [min_Vz]
    type = NodalExtremeValue
    variable = vel_z
    value_type = min
  []
  [max_p]
    type = NodalExtremeValue
    variable = P
    value_type = max
  []
  [min_p]
    type = NodalExtremeValue
    variable = P
    value_type = min
  []
  [max_T]
    type = NodalExtremeValue
    variable = temp
    value_type = max
  []
  [min_T]
    type = NodalExtremeValue
    variable = temp
    value_type = min
  []
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = 'final'
[]
