[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
[]

[UserObjects]
  [x]
    type = LayeredBin
    direction = x
    num_layers = 3
  []
  [vol_avg]
    type = NekBinnedSideIntegral
    bins = 'x'
    field = temperature
    boundary = '5'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
