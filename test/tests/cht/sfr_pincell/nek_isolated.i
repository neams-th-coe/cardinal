[Problem]
  type = NekRSProblem
  casename = 'sfr_pin'

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      initial_flux_integral = 10
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
  boundary = '1'
[]

[ICs]
  [avg_flux]
    type = ConstantIC
    variable = avg_flux
    value = 1.0
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [nek_flux]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
