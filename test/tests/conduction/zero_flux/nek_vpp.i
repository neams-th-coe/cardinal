[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      usrwrk_slot = 0
      direction = to_nek
      conserve_flux_by_sideset = true
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
  boundary = '1 2 3 4 5 6 7 8'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
