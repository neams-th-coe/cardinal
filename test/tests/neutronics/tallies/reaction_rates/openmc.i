[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = ../../heat_source/stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  source_strength = 1e6

  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0

  initial_properties = xml

  [Tallies]
    [Cell]
      type = CellTally
      score = 'absorption fission scatter total'
      blocks = '100 200'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_abs]
    type = ElementIntegralVariablePostprocessor
    variable = absorption
  []
  [fluid_abs]
    type = PointValue
    variable = absorption
    point = '0.0 0.0 2.0'
  []
  [pebble1_abs]
    type = PointValue
    variable = absorption
    point = '0.0 0.0 0.0'
  []
  [pebble2_abs]
    type = PointValue
    variable = absorption
    point = '0.0 0.0 4.0'
  []
  [pebble3_abs]
    type = PointValue
    variable = absorption
    point = '0.0 0.0 8.0'
  []
  [max_err_abs]
    type = TallyRelativeError
    tally_score = 'absorption'
  []

  [total_fis]
    type = ElementIntegralVariablePostprocessor
    variable = fission
  []
  [fluid_fis]
    type = PointValue
    variable = fission
    point = '0.0 0.0 2.0'
  []
  [pebble1_fis]
    type = PointValue
    variable = fission
    point = '0.0 0.0 0.0'
  []
  [pebble2_fis]
    type = PointValue
    variable = fission
    point = '0.0 0.0 4.0'
  []
  [pebble3_fis]
    type = PointValue
    variable = fission
    point = '0.0 0.0 8.0'
  []
  [max_err_fis]
    type = TallyRelativeError
    tally_score = 'fission'
  []

  [total_sca]
    type = ElementIntegralVariablePostprocessor
    variable = scatter
  []
  [fluid_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 2.0'
  []
  [pebble1_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 0.0'
  []
  [pebble2_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 4.0'
  []
  [pebble3_sca]
    type = PointValue
    variable = scatter
    point = '0.0 0.0 8.0'
  []
  [max_err_sca]
    type = TallyRelativeError
    tally_score = 'scatter'
  []

  [total_tot]
    type = ElementIntegralVariablePostprocessor
    variable = total
  []
  [fluid_tot]
    type = PointValue
    variable = total
    point = '0.0 0.0 2.0'
  []
  [pebble1_tot]
    type = PointValue
    variable = total
    point = '0.0 0.0 0.0'
  []
  [pebble2_tot]
    type = PointValue
    variable = total
    point = '0.0 0.0 4.0'
  []
  [pebble3_tot]
    type = PointValue
    variable = total
    point = '0.0 0.0 8.0'
  []
  [max_err_tot]
    type = TallyRelativeError
    tally_score = 'total'
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
