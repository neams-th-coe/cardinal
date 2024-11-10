[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../mesh_tallies/slab.e
  []
  [delete]
    type = BlockDeletionGenerator
    input = file
    block = '2'
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  temperature_blocks = '1'
  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = temp
    temperature_min = 0.0
    temperature_max = 900.0
    n_temperature_bins = 1
  []
[]

[Executioner]
  type = Steady
[]
