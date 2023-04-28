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
  []
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Postprocessors]
  [s1]
    type = NekScalarValuePostprocessor
    userobject = scalar1
  []
[]

[Outputs]
  csv = true
[]
