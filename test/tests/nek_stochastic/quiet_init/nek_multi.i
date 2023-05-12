[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'ethier'
  n_usrwrk_slots = 2

  output = 'scalar02'
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

  # we use the stochastic value to set the scalar (in the .udf files). Here,
  # we are checking that the value of the scalar everywhere is equation to the 's1' postprocessor
  [max_scalar]
    type = ElementExtremeValue
    variable = scalar02
    value_type = max
  []
  [min_scalar]
    type = ElementExtremeValue
    variable = scalar02
    value_type = min
  []
[]

[Outputs]
  csv = true
[]
