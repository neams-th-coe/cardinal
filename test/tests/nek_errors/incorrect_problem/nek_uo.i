[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = dummy
  []
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [subchannel_binning]
    type = HexagonalSubchannelBin
    bundle_pitch = 0.02583914354890463
    pin_pitch = 0.0089656996
    pin_diameter = 7.646e-3
    n_rings = 2
  []
  [vol_avg]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning'
    field = temperature
  []
[]
