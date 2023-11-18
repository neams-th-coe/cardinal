[Mesh]
  [file]
    type = FileMeshGenerator
    file = ../meshes/tet_cube.e
  []
  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  tally_type = cell
  cell_level = 0
  tally_blocks = '1'
  temperature_blocks = '1'
  power = 1000.0
  skinner = moab

  initial_properties = hdf5
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = temp
    n_temperature_bins = 1
    temperature_min = 0.0
    temperature_max = 100.0
  []
[]

[Executioner]
  type = Steady
[]
