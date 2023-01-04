[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
[]

[AuxVariables]
  [solid_temp]
    initial_condition = 800
    blocks = '1 3'
  []
  [fluid_temp]
    initial_condition = 600
    blocks = '2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  solid_blocks = '1 3'
  fluid_blocks = '2'
  tally_blocks = '1'
  tally_type = cell
  solid_cell_level = 1
  fluid_cell_level = 1

  temperature_variables = 'solid_temp fluid_temp'
[]

[Executioner]
  type = Transient
[]
