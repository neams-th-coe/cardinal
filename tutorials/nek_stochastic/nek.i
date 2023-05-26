[Mesh]
  type = NekRSMesh
  boundary = '5'
[]

[Problem]
  type = NekRSProblem
  casename = 'channel'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[UserObjects]
  [k]
    type = NekScalarValue
  []
[]

[Postprocessors]
  [max_temp]
    type = NekVolumeExtremeValue
    field = temperature
  []
  [k_from_stm] # this will just print to the screen the value of k received
    type = NekScalarValuePostprocessor
    userobject = k
  []
  [expect_max_T]
    type = ParsedPostprocessor
    function = '1000.0 / k_from_stm + 500.0'
    pp_names = k_from_stm
  []
[]

[Outputs]
  csv = true
  hide = 'flux_integral'
[]

[Controls]
  [stm]
    type = SamplerReceiver
  []
[]
