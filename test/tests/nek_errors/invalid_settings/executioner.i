[Mesh]
  type = NekRSMesh
  boundary = '3'
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
[]

[Executioner]
  type = Steady
[]
