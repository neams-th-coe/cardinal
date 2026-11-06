[Problem]
  type = NekRSProblem
  casename = 'pyramid'
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
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [two_side1]
    type = NekDiffusiveFluxIntegral
    boundary = '1'
    scalar = "two"
  []
  [flux_side1]
    type = NekDiffusiveFluxIntegral
    boundary = '1'
    scalar = "temperature"
  []
  [flux_side2]
    type = NekDiffusiveFluxIntegral
    boundary = '2'
    scalar = "temperature"
  []
  [flux_side3]
    type = NekDiffusiveFluxIntegral
    boundary = '3'
    scalar = "temperature"
  []
  [flux_side4]
    type = NekDiffusiveFluxIntegral
    boundary = '4'
    scalar = "temperature"
  []
  [flux_side5]
    type = NekDiffusiveFluxIntegral
    boundary = '5'
    scalar = "temperature"
  []
  [flux_side6]
    type = NekDiffusiveFluxIntegral
    boundary = '6'
    scalar = "temperature"
  []
  [flux_side7]
    type = NekDiffusiveFluxIntegral
    boundary = '7'
    scalar = "temperature"
  []
  [flux_side8]
    type = NekDiffusiveFluxIntegral
    boundary = '8'
    scalar = "temperature"
  []
[]
