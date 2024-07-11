[Mesh]
  type = FileMesh
  file = ../../mesh_tallies/slab.e
[]

[AuxVariables]
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
  [material_id]
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
[]

[AuxKernels]
  [temp]
    type = ConstantAux
    variable = temp
    value = 500.0
    execute_on = timestep_begin
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '1 2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  temperature_blocks = '1 2'
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
