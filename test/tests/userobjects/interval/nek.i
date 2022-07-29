[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'

  nondimensional = true
  L_ref = 2.0
  U_ref = 1.0
  rho_0 = 834.5
  Cp_0 = 1228.0
  T_ref = 573.0
  dT_ref = 10.0
[]

[Mesh]
  type = NekRSMesh
  volume = true
  scaling = 2.0
[]

[UserObjects]
  [x_bins]
    type = LayeredBin
    direction = x
    num_layers = 3
  []
  [avg_v]
    type = NekBinnedVolumeAverage
    bins = 'x_bins'
    field = velocity_z
    interval = 3
  []
[]

[VectorPostprocessors]
  [v]
    type = SpatialUserObjectVectorPostprocessor
    userobject = avg_v
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
  interval = 3
[]
