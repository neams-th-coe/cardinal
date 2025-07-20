[Mesh]
  type = NekRSMesh
  boundary = '1 4'
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
  synchronization_interval = parent_app

  [FieldTransfers]
    [heat_flux]
      type = NekBoundaryFlux
      usrwrk_slot = 0
      direction = to_nek
      conserve_flux_by_sideset = true
    []
    [temperature]
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

[Outputs]
  exodus = true
  execute_on = 'final'
[]

[Postprocessors]
  [pin_flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
  [duct_flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '4'
  []
  [max_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
[]
