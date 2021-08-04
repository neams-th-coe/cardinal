[Mesh]
  type = NekRSMesh
  boundary = '6'
[]

[Problem]
  type = NekProblem
  casename = 'brick'
[]

[Executioner]
  type = Transient

  [./TimeStepper]
    type = NekTimeStepper
  [../]
[]

[Outputs]
  exodus = true
  hide = 'flux_integral'
[]
