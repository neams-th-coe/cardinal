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
    fields = 'temp'
    fields_min = '0'
    fields_max = '900'
    n_field_bins = '1'
  []
[]

[Executioner]
  type = Steady
[]
