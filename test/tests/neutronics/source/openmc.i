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
  solid_blocks = '1 2 3'
  tally_blocks = '2 3'
  tally_type = cell
  solid_cell_level = 0
  reuse_source = true
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
