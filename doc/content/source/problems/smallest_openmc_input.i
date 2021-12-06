[Mesh]
  type = FileMesh
  file = pincell.e
[]

[Problem]
  type = OpenMCCellAverageProblem
  solid_blocks = '1 2'
  fluid_blocks = '3'
  tally_blocks = '1'
  power = 1000.0
  tally_type = cell
  solid_cell_level = 0
  fluid_cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
