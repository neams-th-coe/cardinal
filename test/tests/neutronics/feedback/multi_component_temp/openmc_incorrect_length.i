[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
[]

[ICs]
  [solid_temp]
    type = ConstantIC
    variable = solid_temp
    value = 800
    blocks = '1 3'
  []
  [fluid_temp]
    type = ConstantIC
    variable = fluid_temp
    value = 600
    blocks = '2'
  []
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
  cell_level = 1

  temperature_variables = 'solid_temp; fluid_temp'
  temperature_blocks = '1'
[]

[Executioner]
  type = Transient
[]
