[Problem]
  type = NekRSProblem
  casename = 'cylinder'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      usrwrk_slot = 0
      direction = to_nek
      postprocessor_to_conserve = flux_integral
      flux = scalar01_flux
    []
    [temp]
      type = NekFieldVariable
      field = scalar01
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
    field = scalar01
    value_type = max
  []
  [min_temp_nek]
    type = NekVolumeExtremeValue
    field = scalar01
    value_type = min
  []
[]

[Outputs]
  exodus = true
  #execute_on = 'final'
[]
