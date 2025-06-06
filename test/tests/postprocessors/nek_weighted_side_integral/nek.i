[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Mesh]
  type = NekRSMesh
  boundary = '1 2 3 4 5 6'
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
  [weighted_T_side1]
    type = NekMassFluxWeightedSideIntegral
    field = temperature
    boundary = '1'
  []
  [weighted_T_side2]
    type = NekMassFluxWeightedSideIntegral
    field = temperature
    boundary = '2'
  []
  [weighted_T_side3]
    type = NekMassFluxWeightedSideIntegral
    field = temperature
    boundary = '3'
  []
  [weighted_T_side4]
    type = NekMassFluxWeightedSideIntegral
    field = temperature
    boundary = '4'
  []
  [weighted_T_side5]
    type = NekMassFluxWeightedSideIntegral
    field = temperature
    boundary = '5'
  []
  [weighted_T_side6]
    type = NekMassFluxWeightedSideIntegral
    field = temperature
    boundary = '6'
  []
[]
