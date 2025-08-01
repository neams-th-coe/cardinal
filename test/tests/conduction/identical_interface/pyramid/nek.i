[Problem]
  type = NekRSProblem
  casename = 'pyramid'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      usrwrk_slot = 0
      direction = to_nek
      postprocessor_to_conserve = flux_integral
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
  boundary = '2'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_temp_nek]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_temp_nek]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
  [heat_flux]
    type = NekHeatFluxIntegral
    boundary = '2'
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
