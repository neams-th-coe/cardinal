[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0

  assume_separate_tallies = true

  [Tallies]
    [Cell]
      type = CellTally
      block = '1'
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
