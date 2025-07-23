[Mesh]
  type = NekRSMesh
  boundary = '5'
[]

[Problem]
  type = NekRSProblem
  casename = 'channel'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
    []
    [temperature]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []

  [ScalarTransfers]
    [k]
      type = NekScalarValue
      direction = to_nek
      usrwrk_slot = 1
      output_postprocessor = k_from_stm
    []
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_temp]
    type = NekVolumeExtremeValue
    field = temperature
  []
  [k_from_stm] # this will just print to the screen the value of k received
    type = Receiver
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
