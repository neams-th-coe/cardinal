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
      score = 'prompt_nu_fission delayed_nu_fission decay_rate'
      block = '100 200'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_prompt_nu_fission]
    type = ElementIntegralVariablePostprocessor
    variable = prompt_nu_fission
  []
  [fluid_prompt_nu_fission]
    type = PointValue
    variable = prompt_nu_fission
    point = '0.0 0.0 2.0'
  []
  [pebble1_prompt_nu_fission]
    type = PointValue
    variable = prompt_nu_fission
    point = '0.0 0.0 0.0'
  []
  [pebble2_prompt_nu_fission]
    type = PointValue
    variable = prompt_nu_fission
    point = '0.0 0.0 4.0'
  []
  [pebble3_prompt_nu_fission]
    type = PointValue
    variable = prompt_nu_fission
    point = '0.0 0.0 8.0'
  []
  [max_err_prompt_nu_fission]
    type = TallyRelativeError
    tally_score = 'prompt_nu_fission'
  []

  [total_delayed_nu_fission]
    type = ElementIntegralVariablePostprocessor
    variable = delayed_nu_fission
  []
  [fluid_delayed_nu_fission]
    type = PointValue
    variable = delayed_nu_fission
    point = '0.0 0.0 2.0'
  []
  [pebble1_delayed_nu_fission]
    type = PointValue
    variable = delayed_nu_fission
    point = '0.0 0.0 0.0'
  []
  [pebble2_delayed_nu_fission]
    type = PointValue
    variable = delayed_nu_fission
    point = '0.0 0.0 4.0'
  []
  [pebble3_delayed_nu_fission]
    type = PointValue
    variable = delayed_nu_fission
    point = '0.0 0.0 8.0'
  []
  [max_err_delayed_nu_fission]
    type = TallyRelativeError
    tally_score = 'delayed_nu_fission'
  []

  [total_decay_rate]
    type = ElementIntegralVariablePostprocessor
    variable = decay_rate
  []
  [fluid_decay_rate]
    type = PointValue
    variable = decay_rate
    point = '0.0 0.0 2.0'
  []
  [pebble1_decay_rate]
    type = PointValue
    variable = decay_rate
    point = '0.0 0.0 0.0'
  []
  [pebble2_decay_rate]
    type = PointValue
    variable = decay_rate
    point = '0.0 0.0 4.0'
  []
  [pebble3_decay_rate]
    type = PointValue
    variable = decay_rate
    point = '0.0 0.0 8.0'
  []
  [max_err_decay_rate]
    type = TallyRelativeError
    tally_score = 'decay_rate'
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
