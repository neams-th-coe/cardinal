[Mesh]
  type = FileMesh
  file = sphere.e
  allow_renumbering = false
[]

[Problem]
  type = OpenMCProblem
  power = 0.15
  centers = '10 0 0'
  volumes = '14.137166941154067'
  tally_type = mesh
  mesh_template = 'sphere.e'
  pebble_cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
