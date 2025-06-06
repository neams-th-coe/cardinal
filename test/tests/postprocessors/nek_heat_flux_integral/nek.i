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
  [flux_side1]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
  [flux_side2]
    type = NekHeatFluxIntegral
    boundary = '2'
  []
  [flux_side3]
    type = NekHeatFluxIntegral
    boundary = '3'
  []
  [flux_side4]
    type = NekHeatFluxIntegral
    boundary = '4'
  []
  [flux_side5]
    type = NekHeatFluxIntegral
    boundary = '5'
  []
  [flux_side6]
    type = NekHeatFluxIntegral
    boundary = '6'
  []
  [flux_side7]
    type = NekHeatFluxIntegral
    boundary = '7'
  []
  [flux_side8]
    type = NekHeatFluxIntegral
    boundary = '8'
  []
[]
