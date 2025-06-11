[Mesh]
  type = FileMesh
  file = ../../../neutronics/meshes/pincell.e
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0
  temperature_blocks = '2'

  cell_level = 1

  verbose = true
  check_tally_sum = false
  initial_properties = xml

  [Tallies]
    [Cell]
      type = CellTally
      block = '2'
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
