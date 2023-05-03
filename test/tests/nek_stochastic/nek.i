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

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[UserObjects]
  [scalar1]
    type = NekScalarValue
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
