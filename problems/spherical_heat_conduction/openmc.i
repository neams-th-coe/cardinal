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
  power = 0.15 # W
  centers = '0 0 0'
  volumes = '14.137166941154067'
  tally_type = 'mesh'
  pebble_cell_level = 0
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
