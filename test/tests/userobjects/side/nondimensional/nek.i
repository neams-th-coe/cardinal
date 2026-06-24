[Mesh]
  type = NekRSMesh
  scaling = 7.646e-3
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'

  [Dimensionalize]
    L = 7.646e-3
    T = 100.0
    dT = 50.0
    U = 2.0
    rho = 834.5
    Cp = 1228.0
  []

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
    boundary = '4'
  []
  [avg_p]
    type = NekBinnedSideAverage
    bins = 'x y z'
    field = pressure
    boundary = '4'
  []
  [avg_v]
    type = NekBinnedSideAverage
    bins = 'x y z'
    field = velocity
    boundary = '4'
  []
  [integral_T]
    type = NekBinnedSideIntegral
    bins = 'x y z'
    field = temperature
    boundary = '4'
  []
  [integral_p]
    type = NekBinnedSideIntegral
    bins = 'x y z'
    field = pressure
    boundary = '4'
  []
  [integral_v]
    type = NekBinnedSideIntegral
    bins = 'x y z'
    field = velocity
    boundary = '4'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]


# The data is passed to a sub-application just to keep the gold file smaller, since
# we are only interested in the data on the boundary anwyays

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = 'sub.i'
    sub_cycling = true
    execute_on = timestep_end
  []
[]

[Transfers]
  [u1]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = avg_T
    source_user_object = avg_T
    to_multi_app = sub
  []
  [u2]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = avg_p
    source_user_object = avg_p
    to_multi_app = sub
  []
  [u3]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = avg_v
    source_user_object = avg_v
    to_multi_app = sub
  []
  [u4]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = integral_T
    source_user_object = integral_T
    to_multi_app = sub
  []
  [u5]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = integral_p
    source_user_object = integral_p
    to_multi_app = sub
  []
  [u6]
    type = MultiAppGeneralFieldUserObjectTransfer
    variable = integral_v
    source_user_object = integral_v
    to_multi_app = sub
  []
[]

[Outputs]
  exodus = true
[]
