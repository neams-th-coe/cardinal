[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  cell_level = 0
  tally_type = none
  initial_properties = hdf5
[]

[Executioner]
  type = Transient
[]
