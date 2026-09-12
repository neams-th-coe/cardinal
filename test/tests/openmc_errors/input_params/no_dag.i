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
    fields = 'temp'
    fields_min = '0.0'
    fields_max = '1000'
    n_field_bins = '5'
  []
[]

[Executioner]
  type = Transient
[]
