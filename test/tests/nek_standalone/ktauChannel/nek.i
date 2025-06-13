[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'channel'

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

[Outputs]
  exodus = true
[]

[Postprocessors]
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
  []
  [T_at_point]
    type = PointValue
    variable = temp
    point = '4.0 -0.5 0.0'
  []
  [pct_change]
    type = PercentChangePostprocessor
    postprocessor = max_T
  []
[]

[UserObjects]
  [layered_average]
    type = LayeredAverage
    direction = x
    variable = P
    num_layers = 4
  []
[]

[AuxVariables]
  [layered_p]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [layered_p]
    type = SpatialUserObjectAux
    variable = layered_p
    user_object = layered_average
  []
[]
