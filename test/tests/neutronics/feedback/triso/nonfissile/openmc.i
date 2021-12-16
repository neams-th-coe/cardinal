[Mesh]
  [solid]
    type = FileMeshGenerator
    file = ../solid.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem

  power = 100.0
  scaling = 100.0
  solid_blocks = '1 2'
  tally_blocks = '2'
  tally_type = cell
  solid_cell_level = 1
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
