[Mesh]
  type = NekRSMesh
  boundary = '1'
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'pin'
  write_fld_files = true

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = flux_integral
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
[]

[Postprocessors]
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
  []
  [avg_T]
    type = NekVolumeAverage
    field = temperature
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
  csv = true
  execute_on = 'final'

  # just to make the gold files smaller
  hide = 'avg_flux flux_integral'
[]
