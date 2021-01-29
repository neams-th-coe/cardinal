[Mesh]
  type = NekRSMesh
  boundary = '6'
[]

[Problem]
  type = NekProblem
[]

[Executioner]
  type = Transient

  [./TimeStepper]
    type = NekTimeStepper
  [../]
[]

[Postprocessors]
  [flux_integral]
    type = Receiver
    default = 0
  []
[]

[Outputs]
  exodus = true
  hide = 'flux_integral'
[]
