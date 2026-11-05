[Problem]
  type = NekRSProblem
  casename = 'cylinder2'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      usrwrk_slot = 0
      direction = to_nek
      postprocessor_to_conserve = flux_integral
      scalar = "other"
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

[Outputs]
  exodus = true
  csv = true
[]
