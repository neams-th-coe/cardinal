[Mesh]
  type = NekMesh
[]

[Variables]
  [dummy]
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
  [exo]
    type = Exodus
    output_dimension = 3
  []
[]
