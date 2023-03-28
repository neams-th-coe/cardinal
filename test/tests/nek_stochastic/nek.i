[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'ethier'
  n_usrwrk_slots = 2
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[UserObjects]
  [scalar1]
    type = NekScalarValue
    value = 1
    usrwrk_slot = 0
  []
  [scalar2]
    type = NekScalarValue
    value = 2
    usrwrk_slot = 0
  []
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]
