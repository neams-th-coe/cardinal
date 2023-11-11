[Mesh]
  type = FileMesh
  file = pincell.e
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 1000.0

  temperature_blocks = '1 2 3'
  density_blocks = '3'

  tally_type = cell
  tally_blocks = '1'
  cell_level = 0
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
