[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSSeparateDomainProblem
  casename = 'brick'
  coupling_type = 'inlet outlet'
  outlet_boundary = '2'
  inlet_boundary = '1'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
