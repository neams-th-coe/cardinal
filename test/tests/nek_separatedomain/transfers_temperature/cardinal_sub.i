[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Postprocessors]
  [toNekRS_velocity]
    type = Receiver
    default = 0.1
  []
  [toNekRS_temperature]
    type = Receiver
    default = 1.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
