[Problem]
  type = NekRSProblem
  casename = 'brick'

  [Dimensionalize]
    L = 2.0
    U = 1.0
    rho = 834.5
    Cp = 1228.0
    T = 573.0
    dT = 10.0
  []
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
