[Mesh]
  type = FileMesh
  file = ../../mesh_tallies/slab.e
[]

x0 = 12.5
x1 = 37.5
x2 = 62.5

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
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
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
  []
  [material_id]
    type = CellMaterialIDAux
    variable = material_id
  []
  [cell_density]
    type = CellDensityAux
    variable = cell_density
  []
[]

T0 = 600.0
dT = 50.0

[Functions]
  [density]
    type = ParsedFunction
    expression = 'if (x <= ${x0}, ${fparse T0 - dT}, if (x <= ${x1}, ${T0}, if (x <= ${x2}, ${fparse T0 + dT}, ${fparse T0 + 2 * dT})))'
  []
[]

[ICs]
  [temp]
    type = ConstantIC
    variable = temp
    value = 500.0
  []
[]

[AuxKernels]
  [density]
    type = FunctionAux
    variable = density
    function = density
    execute_on = timestep_begin
  []
[]

[Tallies]
  [Mesh]
    type = CellTally
    tally_blocks = '1 2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  temperature_blocks = '1 2'
  density_blocks = '1 2'
  cell_level = 0
  power = 100.0
[]

[Postprocessors]
  [k]
    type = KEigenvalue
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
