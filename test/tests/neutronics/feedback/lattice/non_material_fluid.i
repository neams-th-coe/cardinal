[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0

  temperature_blocks = '2'
  density_blocks = '2'

  cell_level = 0

  tally_type = mesh
  mesh_template = ../../meshes/pincell.e
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
