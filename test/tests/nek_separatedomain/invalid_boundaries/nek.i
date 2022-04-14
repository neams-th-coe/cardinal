[Problem]
  type = NekRSSeparateDomainProblem
  coupling_type = 'inlet'

  inlet_boundary  = '1'
  outlet_boundary = '2'

  casename = 'pyramid'
[]

[Mesh]
  type = NekRSMesh
  boundary = 1
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

