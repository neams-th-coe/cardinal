[Mesh]
  type = FileMesh
  file = ../../../neutronics/meshes/pincell.e
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  solid_blocks = '2'
  tally_type = cell
  tally_blocks = '2'

  solid_cell_level = 1

  verbose = true
  check_tally_sum = false
  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
