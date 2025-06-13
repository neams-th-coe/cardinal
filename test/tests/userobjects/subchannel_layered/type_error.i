[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
[]

[UserObjects]
  [dummy]
    type = NearestNodeNumberUO
    point = '0.0 0.0 0.1'
  []
  [vol_avg]
    type = NekBinnedVolumeAverage
    bins = 'dummy'
    field = temperature
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
