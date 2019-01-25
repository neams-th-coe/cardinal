[Mesh]
  type = NekMesh
[]

[Variables]
  [temp]
  []
[]

[Problem]
  type = NekProblem
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  exodus = true
[]
