[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
[]

[ICs]
  [density1]
    type = ConstantIC
    variable = density1
    value = 800
    blocks = '1 3'
  []
  [density2]
    type = ConstantIC
    variable = density2
    value = 600
    blocks = '2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  tally_blocks = '1'
  tally_type = cell
  cell_level = 1

  density_variables = 'density1; density2'
  density_blocks = '1'
  temperature_blocks = '1'
[]

[Executioner]
  type = Transient
[]
