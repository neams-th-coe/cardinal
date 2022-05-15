[Mesh]
  [pincell]
    type = FileMeshGenerator
    file = ../../meshes/pincell.e
  []
  [delete_block]
    type = BlockDeletionGenerator
    input = pincell
    block = 2
  []
  [add_surrounding]
    type = FileMeshGenerator
    file = surrounding.e
  []
  [translate_surrounding]
    type = TransformGenerator
    input = add_surrounding
    transform = translate
    vector_value = '0.0 0.0 5.0'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'delete_block translate_surrounding'
  []
[]

[AuxVariables]
  [cell_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_instance]
    family = MONOMIAL
    order = CONSTANT
  []
  [cell_temperature]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [cell_id]
    type = CellIDAux
    variable = cell_id
  []
  [cell_instance]
    type = CellInstanceAux
    variable = cell_instance
  []
  [cell_temperature]
    type = CellTemperatureAux
    variable = cell_temperature
    execute_on = 'timestep_end'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 500.0

  # In the OpenMC model script, we set the fuel temperature to 500, the clad temperature
  # to 450, and the exterior temperature to 300. Checking that the temperatures obtained with
  # CellTemperatureAux match these values will confirm that the correct cell level is
  # fetched in the exterior region, which is at a level of 0 (greater than the lowest_solid_cell_level).
  initial_properties = xml

  # block 5 should map to an OpenMC region on level 0 in the geometry; by setting
  # the lowest cell level, we will just map to level 0 in this region instead of 1.
  solid_blocks = '1 3 5'
  lowest_solid_cell_level = 1

  tally_blocks = '1'
  verbose = true
  tally_type = cell
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true

  # makes the gold files smaller, since these are not of concern for the test
  hide = 'temp heat_source'
[]
