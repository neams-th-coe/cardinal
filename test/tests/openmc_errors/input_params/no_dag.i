[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '1'
  cell_level = 0
  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = temp
    temperature_max = 1000.0
    n_temperature_bins = 5
  []
[]

[Executioner]
  type = Transient
[]
