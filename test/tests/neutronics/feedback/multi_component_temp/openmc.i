[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
  parallel_type = replicated
[]

[ICs]
  [solid_temp]
    type = ConstantIC
    variable = solid_temp
    value = 800
  []
  [fluid_temp]
    type = ConstantIC
    variable = fluid_temp
    value = 600
  []
[]

solid_blocks = '1 3'

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  solid_blocks = '1 3'
  fluid_blocks = '2'
  tally_blocks = '1'
  tally_type = cell
  solid_cell_level = 1
  fluid_cell_level = 1
  skip_first_incoming_transfer = true

  temperature_variables = 'solid_temp solid_temp fluid_temp'
  temperature_blocks = '${solid_blocks} 2'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  hide = 'heat_source'
[]
