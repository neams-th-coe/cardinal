[Mesh]
  type = NekRSMesh
  boundary = '1 2'
[]

[Problem]
  type = NekRSSeparateDomainProblem
  casename = 'brick'
  coupling_type = 'inlet outlet'
  inlet_boundary = '1'
  outlet_boundary = '2'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
