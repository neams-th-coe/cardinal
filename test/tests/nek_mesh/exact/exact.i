[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
[]

[Mesh]
  type = NekRSMesh
  exact = true
  boundary = '1 3'
[]
