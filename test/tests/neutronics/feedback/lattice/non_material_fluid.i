[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
  allow_renumbering = false
[]

[ICs]
  [density]
    type = ConstantIC
    variable = density
    value = 1000.0
  []
  [temp]
    type = ConstantIC
    variable = temp
    value = 1000.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  fluid_blocks = '2'
  tally_type = mesh
  fluid_cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
