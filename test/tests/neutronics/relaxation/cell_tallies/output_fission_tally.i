[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = ../../meshes/sphere_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.06
                 0 0 0.10'
  []
  [set_block_ids]
    type = SubdomainIDGenerator
    input = repeat
    subdomain_id = 0
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
  [temp]
    type = FunctionAux
    variable = temp
    function = axial
    execute_on = initial
  []
[]

[Functions]
  [axial]
    type = ParsedFunction
    value = '500 + z / 0.10 * 100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  solid_blocks = '0'
  tally_blocks = '0'
  tally_type = cell
  tally_name = heat_source
  solid_cell_level = 1
  scaling = 100.0

  output = 'unrelaxed_tally'
  relaxation = constant
[]

# These two auxvariables and auxkernels are only here to avoid a re-gold (due to a
# variable name change). The 'dummy' auxvariable is only here due to MOOSE issue #17534
[AuxVariables]
  [fission_tally]
    family = MONOMIAL
    order = CONSTANT
  []
  [dummy]
  []
[]

[AuxKernels]
  [copy]
    type = SelfAux
    variable = fission_tally
    v = heat_source_raw
  []
  [dummy]
    type = ConstantAux
    variable = dummy
    value = 0.0
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
  hide = 'cell_temperature dummy heat_source_raw'
[]
