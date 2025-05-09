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
  source_strength = 1e6
  verbose = true

  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0

  initial_properties = xml

  max_batches = 200

  [Tallies]
    [Cell]
      type = CellTally
      score = 'H3_production'
      block = '100 200'
      trigger = 'rel_err'
      # If tally bins with zeros are ignored, this will fire immediately since
      # the fluid region doesn't have a tritium production score.
      trigger_threshold = '1.0'
      trigger_ignore_zeros = 'true'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [max_err]
    type = TallyRelativeError
    tally_score = 'H3_production'
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
