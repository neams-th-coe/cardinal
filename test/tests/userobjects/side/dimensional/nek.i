[GlobalParams]
  check_boundary_restricted = false
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'

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

[AuxVariables]
  [avg_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_v]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_v]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [avg_T]
    type = SpatialUserObjectAux
    variable = avg_T
    user_object = avg_T
    boundary = '2'
  []
  [avg_p]
    type = SpatialUserObjectAux
    variable = avg_p
    user_object = avg_p
    boundary = '2'
  []
  [avg_v]
    type = SpatialUserObjectAux
    variable = avg_v
    user_object = avg_v
    boundary = '2'
  []
  [integral_T]
    type = SpatialUserObjectAux
    variable = integral_T
    user_object = integral_T
    boundary = '2'
  []
  [integral_p]
    type = SpatialUserObjectAux
    variable = integral_p
    user_object = integral_p
    boundary = '2'
  []
  [integral_v]
    type = SpatialUserObjectAux
    variable = integral_v
    user_object = integral_v
    boundary = '2'
  []
[]

[UserObjects]
  [x]
    type = LayeredBin
    direction = x
    num_layers = 2
  []
  [y]
    type = LayeredBin
    direction = y
    num_layers = 2
  []
  [z]
    type = LayeredBin
    direction = z
    num_layers = 3
  []
  [avg_T]
    type = NekBinnedSideAverage
    bins = 'x y z'
    field = temperature
    boundary = '2'
  []
  [avg_p]
    type = NekBinnedSideAverage
    bins = 'x y z'
    field = pressure
    boundary = '2'
  []
  [avg_v]
    type = NekBinnedSideAverage
    bins = 'x y z'
    field = velocity
    boundary = '2'
  []
  [integral_T]
    type = NekBinnedSideIntegral
    bins = 'x y z'
    field = temperature
    boundary = '2'
  []
  [integral_p]
    type = NekBinnedSideIntegral
    bins = 'x y z'
    field = pressure
    boundary = '2'
  []
  [integral_v]
    type = NekBinnedSideIntegral
    bins = 'x y z'
    field = velocity
    boundary = '2'
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
  file_base = nek_dim
[]
