[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  boundary = '3'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  [out]
    type = CSV
    hide = 'flux_integral'
    execute_on = 'final'
  []
[]

[Postprocessors]
  [Re]
    type = ReynoldsNumber
    L_ref = 0.25
    boundary = '1'
  []
  [area]
    type = NekSideIntegral
    field = unity
    boundary = '1'
  []
  [mdot]
    type = NekMassFluxWeightedSideIntegral
    field = unity
    boundary = '1'
  []
  [inlet_v]
    type = NekSideAverage
    field = velocity
    boundary = '1'
  []
[]
