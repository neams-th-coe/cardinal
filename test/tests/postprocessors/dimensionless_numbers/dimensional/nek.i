[Problem]
  type = NekRSProblem
  casename = 'brick'
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
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [Re]
    type = ReynoldsNumber
    L_ref = 0.25
    boundary = '1'
  []
  [Pe]
    type = PecletNumber
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
