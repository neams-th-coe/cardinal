[Mesh]
  type = FileMesh
  file = sphere.e
[]

[Variables]
  [dummy]
  []
[]

[Problem]
  type = OpenMCProblem
  power = 0.15
  centers = '0 0 0'
  volumes = '0.0002782618569027357'
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
