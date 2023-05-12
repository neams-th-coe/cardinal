[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'pyramid'
  n_usrwrk_slots = 1
  output = 'temperature'
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

  # we use the stochastic value to set the boundary condition on temperature on sideset 1;
  # we are checking that the value of the temperature on sideset 1 indeed matches that value
  [max_on_side]
    type = NekSideExtremeValue
    field = temperature
    boundary = '1'
    value_type = max
  []
  [min_on_side]
    type = NekSideExtremeValue
    field = temperature
    boundary = '1'
    value_type = min
  []
[]

[Outputs]
  csv = true
[]
