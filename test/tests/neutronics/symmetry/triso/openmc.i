[Mesh]
  [solid]
    type = FileMeshGenerator
    file = solid_mesh_in.e
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
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp
    block = 'graphite'
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
    expression = '500+(exp(10*x)+exp(10*y))*50'
  []
  [temp_fuel]
    type = ParsedFunction
    expression = '500+(exp(10*x)+exp(10*y))*50 + 100'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = 'compacts'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  identical_cell_fills = 'compacts'
  check_identical_cell_fills = true
  check_equal_mapped_tally_volumes = true

  power = 1000.0
  scaling = 100.0
  temperature_blocks = 'graphite compacts'
  cell_level = 1

  symmetry_mapper = sym
[]

[UserObjects]
  [sym]
    type = SymmetryPointGenerator
    normal = '-1.0 0.0 0.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]

[Outputs]
  exodus = true
  # it is hard to get parallel-reproducible results with high-scattering TRISO
  # problems, so this input will only check on the mapping
  hide = 'temp kappa_fission'
[]
