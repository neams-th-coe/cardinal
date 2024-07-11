[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../meshes/tet_cube.e
  []
  parallel_type = replicated
[]

[Tallies]
  [Mesh]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  temperature_blocks = '1'
  power = 1000.0
  skinner = moab
[]

[UserObjects]
  [moab]
    type = NearestNodeNumberUO
    point = '0.0 0.0 0.0'
  []
[]

[Executioner]
  type = Steady
[]
