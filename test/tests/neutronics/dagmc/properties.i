[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../meshes/tet_cube.e
  []
  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  cell_level = 0
  temperature_blocks = '1'
  power = 1000.0
  skinner = moab

  initial_properties = hdf5

  [Tallies]
    [Cell]
      type = CellTally
      block = '1'
    []
  []
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    fields = 'temp'
    fields_min = '0'
    fields_max = '100'
    n_field_bins = '1'
  []
[]

[Executioner]
  type = Steady
[]
