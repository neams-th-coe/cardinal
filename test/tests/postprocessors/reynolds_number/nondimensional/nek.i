[Problem]
  type = NekRSProblem

  nondimensional = true
  L_ref = 0.25
  U_ref = 0.001
  rho_0 = 834.5
  Cp_0 = 1228.0
  T_ref = 573.0
  dT_ref = 10.0
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
  [out]
    type = CSV
    hide = 'flux_integral'
    execute_on = 'final'
  []
[]

[Postprocessors]
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
