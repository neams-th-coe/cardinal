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
  [y_bins]
    type = LayeredBin
    direction = y
    num_layers = 3
  []
  [x_bins2]
    type = LayeredBin
    direction = x
    num_layers = 12
  []

  # should error because weve specified two x-direction bins
  [vol_integral]
    type = NekBinnedVolumeIntegral
    bins = 'x_bins y_bins x_bins2'
    field = unity
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
