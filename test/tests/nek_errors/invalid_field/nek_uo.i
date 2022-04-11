[Problem]
  type = NekRSStandaloneProblem
  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[UserObjects]
  [subchannel_binning]
    type = HexagonalSubchannelBin
    bundle_pitch = 10
    pin_pitch = 1
    pin_diameter = 1
    n_rings = 1
  []
  [vol_avg]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning'
    field = temperature
  []
[]
