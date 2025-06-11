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
      score = 'inverse_velocity nu_fission nu_scatter'
      block = '100 200'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_nu_scatter]
    type = ElementIntegralVariablePostprocessor
    variable = nu_scatter
  []
  [fluid_nu_scatter]
    type = PointValue
    variable = nu_scatter
    point = '0.0 0.0 2.0'
  []
  [pebble1_nu_scatter]
    type = PointValue
    variable = nu_scatter
    point = '0.0 0.0 0.0'
  []
  [pebble2_nu_scatter]
    type = PointValue
    variable = nu_scatter
    point = '0.0 0.0 4.0'
  []
  [pebble3_nu_scatter]
    type = PointValue
    variable = nu_scatter
    point = '0.0 0.0 8.0'
  []
  [max_err_nu_scatter]
    type = TallyRelativeError
    tally_score = 'nu_scatter'
  []

  [total_nu_fission]
    type = ElementIntegralVariablePostprocessor
    variable = nu_fission
  []
  [fluid_nu_fission]
    type = PointValue
    variable = nu_fission
    point = '0.0 0.0 2.0'
  []
  [pebble1_nu_fission]
    type = PointValue
    variable = nu_fission
    point = '0.0 0.0 0.0'
  []
  [pebble2_nu_fission]
    type = PointValue
    variable = nu_fission
    point = '0.0 0.0 4.0'
  []
  [pebble3_nu_fission]
    type = PointValue
    variable = nu_fission
    point = '0.0 0.0 8.0'
  []
  [max_err_nu_fission]
    type = TallyRelativeError
    tally_score = 'nu_fission'
  []

  [total_inverse_velocity]
    type = ElementIntegralVariablePostprocessor
    variable = inverse_velocity
  []
  [fluid_inverse_velocity]
    type = PointValue
    variable = inverse_velocity
    point = '0.0 0.0 2.0'
  []
  [pebble1_inverse_velocity]
    type = PointValue
    variable = inverse_velocity
    point = '0.0 0.0 0.0'
  []
  [pebble2_inverse_velocity]
    type = PointValue
    variable = inverse_velocity
    point = '0.0 0.0 4.0'
  []
  [pebble3_inverse_velocity]
    type = PointValue
    variable = inverse_velocity
    point = '0.0 0.0 8.0'
  []
  [max_err_inverse_velocity]
    type = TallyRelativeError
    tally_score = 'inverse_velocity'
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
