[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
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

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  solid_blocks = '1 3'
  fluid_blocks = '2'
  tally_blocks = '1'
  tally_type = cell
  solid_cell_level = 1
  fluid_cell_level = 1
  initial_properties = xml

  temperature_variables = 'solid_temp solid_temp fluid_temp'
  temperature_blocks = '1 3 2'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  hide = 'kappa_fission'
[]
