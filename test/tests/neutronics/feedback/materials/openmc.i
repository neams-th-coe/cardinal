[Mesh]
  type = FileMesh
  file = ../../meshes/pincell.e
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = temp
  []
  [density]
    type = FunctionIC
    variable = density
    function = density
  []
[]

[Functions]
  [temp]
    type = ParsedFunction
    expression = '500'
  []
  [density]
    type = ParsedFunction
    expression = '800+z*10'
  []
[]

[AuxVariables]
  [material_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_density]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
    execute_on = 'timestep_end'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 500.0
  temperature_blocks = '1 2 3'
  density_blocks = '2'

  cell_level = 0

  map_density_by_cell = false
[]

[Postprocessors]
  [avg_density]
    type = ElementAverageValue
    variable = density
    block = '2'
  []
  [rho1]
    type = PointValue
    variable = cell_density
    point = '0.5 0 1.5'
  []
  [rho2]
    type = PointValue
    variable = cell_density
    point = '0.5 0 8.5'
  []
  [mat1]
    type = PointValue
    variable = material_id
    point = '0.5 0 1.5'
  []
  [mat2]
    type = PointValue
    variable = material_id
    point = '0.5 0 8.5'
  []
  [k]
    type = KEigenvalue
    value_type = collision
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
