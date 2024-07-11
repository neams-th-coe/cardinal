[Mesh]
  [pebble]
    type = FileMeshGenerator
    file = ../../meshes/sphere_in_m.e
  []
  [repeat]
    type = CombinerGenerator
    inputs = pebble
    positions = '0 0 0.02
                 0 0 0.10'
  []
  [set_block_ids]
    type = SubdomainIDGenerator
    input = repeat
    subdomain_id = 0
  []
[]

[AuxKernels]
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
    expression = '500 + z / 0.10 * 100'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_score = 'kappa_fission heating'
    tally_name = 'heat_source heating'
    tally_blocks = '0'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1500.0
  temperature_blocks = '0'
  cell_level = 1
  scaling = 100.0

  # our problem is missing overlap for fissile regions, so our local and global tallies
  # wont match
  check_tally_sum = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [kf]
    type = ElementIntegralVariablePostprocessor
    variable = heat_source
  []
  [peb1_kf]
    type = PointValue
    variable = heat_source
    point = '0 0 0.02'
  []
  [peb2_kf]
    type = PointValue
    variable = heat_source
    point = '0 0 0.10'
  []
  [heating]
    type = ElementIntegralVariablePostprocessor
    variable = heating
  []
  [peb1_ht]
    type = PointValue
    variable = heating
    point = '0 0 0.02'
  []
  [peb2_ht]
    type = PointValue
    variable = heating
    point = '0 0 0.10'
  []
[]
