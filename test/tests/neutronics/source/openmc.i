[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 600.0
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = ${fparse 3000e6 / 273 / (17 * 17)}
  temperature_blocks = '1 2 3'
  cell_level = 0
  reuse_source = true

  [Tallies]
    [Cell]
      type = CellTally
      block = '2 3'
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
