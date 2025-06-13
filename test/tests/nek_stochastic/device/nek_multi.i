[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 1

  [ScalarTransfers]
    [scalar1]
      type = NekScalarValue
      direction = to_nek
      usrwrk_slot = 0
      output_postprocessor = s1
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
