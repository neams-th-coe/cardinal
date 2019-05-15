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
  power = 0.3
  centers = '0 0 0 0 0 3.15'
  volumes = '0.0335 0.0335'
[]

[Executioner]
  type = Transient
  num_steps = 50
[]

[Outputs]
  [exo]
    type = Exodus
    output_dimension = 3
  []
[]
