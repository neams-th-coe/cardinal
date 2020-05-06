[Mesh]
  type = FileMesh
  file = two_sphere.e
[]

[Variables]
  [dummy]
  []
[]

[Problem]
  type = OpenMCProblem
  power = 1000
  centers = '0 0 0 0 0 3.15'
  volumes = '14.137166941154067 14.137166941154067'
  tally_type = 'cell'
  mesh_template = 'sphere.e'
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
