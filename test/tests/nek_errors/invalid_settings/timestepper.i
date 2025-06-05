[Mesh]
  type = NekRSMesh
  boundary = '3'
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
[]

[Executioner]
  type = Transient
[]
