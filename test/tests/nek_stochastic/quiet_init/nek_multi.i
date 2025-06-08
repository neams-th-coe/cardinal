[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'ethier'
  n_usrwrk_slots = 2

  [ScalarTransfers]
    [scalar1]
      type = NekScalarValue
      direction = to_nek
      usrwrk_slot = 0
      output_postprocessor = s1
    []
  []

  [FieldTransfers]
    [scalar02]
      type = NekFieldVariable
      direction = from_nek
    []
  []
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

[Postprocessors]
  [s1]
    type = Receiver
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
