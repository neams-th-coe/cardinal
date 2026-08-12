[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
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
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0

  max_batches = 200

  [Tallies]
    [Cell]
      type = CellTally
      block = '100'
      trigger = rel_err
      trigger_threshold = 2e-2

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_tally_err]
    type = TallyRelativeError
  []

  [threshold]
    type = ParsedPostprocessor
    expression = 'if (max_tally_err < th, 1, 0)'
    pp_names = 'max_tally_err'
    constant_names = 'th'
    constant_expressions = '2e-2'
  []
[]

[Outputs]
  csv = true
  hide = 'max_tally_err'
  execute_on = 'TIMESTEP_END'
[]
