# Warn when initial_properties = xml is combined with settings.properties_file.

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
  initial_properties = xml
  xml_directory = xml_and_properties
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
