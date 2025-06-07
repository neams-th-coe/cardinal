[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[AuxVariables]
  [uo]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [uo]
    type = NekSpatialBinComponentAux
    variable = uo
    user_object = vol_integral
    component = 0
  []
[]

[UserObjects]
  [x_bins]
    type = LayeredBin
    direction = x
    num_layers = 3
  []
  [vol_integral]
    type = NekBinnedVolumeIntegral
    bins = 'x_bins'
    field = pressure
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
