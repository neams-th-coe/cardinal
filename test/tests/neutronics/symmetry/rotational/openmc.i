[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid_mesh_in.e
  []
[]

[AuxVariables]
  [cell_instance]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_instance]
    type = CellInstanceAux
    variable = cell_instance
  []
  [cell_id]
    type = CellIDAux
    variable = cell_id
  []
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp
    block = 'graphite 200'
  []
  [temp_fuel]
    type = FunctionIC
    variable = temp
    function = temp_fuel
    block = 'compacts'
  []
[]

# We set an initial temperature distribution that is NOT symmetry about the imposed
# symmmetry plane; by checking the cell temperature, we can ensure that the data
# mapping is done correctly (in addition to checking that the *extracted* heat source
# reflects the imposed symmetry)
[Functions]
  [temp]
    type = ParsedFunction
    value = '500+(exp(10*x)+exp(10*y))*50'
  []
  [temp_fuel]
    type = ParsedFunction
    value = '500+(exp(10*x)+exp(10*y))*50 + 100'
  []
[]


[Problem]
  type = OpenMCCellAverageProblem

  power = 1000.0
  scaling = 100.0
  solid_blocks = '1 2 200'
  tally_blocks = '2'
  tally_type = cell
  solid_cell_level = 1

  symmetry_plane_normal = '${fparse -sqrt(3.0) / 2.0} 0.5 0.0'
  symmetry_axis = '0.0 0.0 1.0'
  symmetry_angle = 60.0
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []

  # check a few of the pins to be sure that the reflected heat source matches
  [pin]
    type = PointValue
    variable = heat_source
    point = '0.097 0.097 4.0'
  []

  [pin1]
    type = PointValue
    variable = heat_source
    point = '0.036 0.13 4.0'
  []
  [diff1]
    type = DifferencePostprocessor
    value1 = pin1
    value2 = pin
  []

  [pin2]
    type = PointValue
    variable = heat_source
    point = '-0.13 0.0313 4.0'
  []
  [diff2]
    type = DifferencePostprocessor
    value1 = pin2
    value2 = pin
  []

  [pin3]
    type = PointValue
    variable = heat_source
    point = '-0.13 -0.032 4.0'
  []
  [diff3]
    type = DifferencePostprocessor
    value1 = pin3
    value2 = pin
  []

  [pin4]
    type = PointValue
    variable = heat_source
    point = '0.039 -0.13 4.0'
  []
  [diff4]
    type = DifferencePostprocessor
    value1 = pin4
    value2 = pin
  []

  [pin5]
    type = PointValue
    variable = heat_source
    point = '0.09 -0.1 4.0'
  []
  [diff5]
    type = DifferencePostprocessor
    value1 = pin5
    value2 = pin
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  hide = 'temp pin pin1 pin2 pin3 pin4 pin5 heat_source'
[]
