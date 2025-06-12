[Problem]
  type = NekRSProblem
  casename = 'pyramid/pyramid'
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
  [out]
    type = CSV
    execute_on = 'final'
  []
[]


[Postprocessors]
  [temp_avg1]
    type = NekSideAverage
    field = temperature
    boundary = '1'
  []
[]
