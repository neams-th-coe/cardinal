[Mesh]
  type = FileMesh
  file = ../../../neutronics/meshes/pincell.e
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  temperature_blocks = '1 2 3'
  density_blocks = '2'
  verbose = true
  cell_level = 1
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
