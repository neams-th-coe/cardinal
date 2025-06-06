[Problem]
  type = NekRSProblem
  casename = 'brick'

  [Dimensionalize]
    L = 0.25
    U = 0.001
    rho = 834.5
    Cp = 1228.0
    T = 573.0
    dT = 10.0
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '3'
  scaling = 0.25
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
  [Pe]
    type = PecletNumber
    boundary = '1'
  []
  [Re]
    type = ReynoldsNumber
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
