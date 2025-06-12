[Problem]
  type = NekRSProblem
  casename = 'cube'
  n_usrwrk_slots = 2

  [FieldTransfers]
    [heat_source]
      type = NekVolumetricSource
      direction = to_nek
      postprocessor_to_conserve = source_integral
      usrwrk_slot = 0
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  order = SECOND
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [flux_out]
    type = NekHeatFluxIntegral
    boundary = '1 2 3 4 5 6'
  []
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [avg_T_volume]
    type = NekVolumeIntegral
    field = temperature
  []
  [avg_T_back]
    type = NekSideAverage
    field = temperature
    boundary = '6'
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'

  # for this tests purposes, we only want to check temperature. This keeps the gold file smaller
  hide = 'heat_source'
[]
