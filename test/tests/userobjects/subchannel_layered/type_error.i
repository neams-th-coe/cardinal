[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'sfr_7pin'
  output = 'temperature'
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

[Outputs]
  exodus = true
[]
