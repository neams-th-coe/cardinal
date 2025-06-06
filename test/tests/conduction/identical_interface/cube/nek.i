[Problem]
  type = NekRSProblem
  casename = 'cube'

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
  boundary = '4'
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
