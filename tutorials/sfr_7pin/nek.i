interval = 100

[Mesh]
  type = NekRSMesh
  boundary = '1 4'
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
  synchronization_interval = parent_app
  n_usrwrk_slots = 1

  [FieldTransfers]
    [heat_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
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

  # this will only display the NekRS output every N time steps; postprocessors
  # are still computed on every step, just not output to the console
  time_step_interval = ${interval}
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
