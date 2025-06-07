[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../meshes/tet_cube.e
  []
  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  temperature_blocks = '1'
  power = 1000.0
  skinner = moab

  [Tallies]
    [Mesh]
      type = CellTally
      block = '1'
    []
  []
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
