[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = pipe
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [avg_yp]
    type = NekYPlus
    boundary = '1'
    mesh = 'fluid'
    value_type = 'avg'
  []
  [min_yp]
    type = NekYPlus
    boundary = '1'
    mesh = 'fluid'
    value_type = 'min'
  []
  [max_yp]
    type = NekYPlus
    boundary = '1'
    mesh = 'fluid'
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]
