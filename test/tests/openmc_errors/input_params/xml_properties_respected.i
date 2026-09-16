# Confirm that settings.properties_file temperatures are used when
# initial_properties = xml. The XML temperature_default is 600 K; the
# properties file stores 900 K.

[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
    execute_on = 'timestep_end'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '1'
  cell_level = 0
  initial_properties = xml
  xml_directory = 'xml_and_properties'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [T_cell]
    type = ElementAverageValue
    variable = cell_temperature
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]
