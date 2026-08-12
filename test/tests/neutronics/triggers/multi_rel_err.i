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
  batches = 20
  cell_level = 0

  source_rate_normalization = 'kappa_fission'

  max_batches = 200

  [Tallies]
    [Cell]
      type = CellTally
      block = '100'
      score = 'damage_energy kappa_fission'

      trigger = 'rel_err rel_err'
      trigger_threshold = '2e-2 2e-2'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_err_kf]
    type = TallyRelativeError
    tally_score = 'kappa_fission'
  []
  [max_err_de]
    type = TallyRelativeError
    tally_score = 'damage_energy'
  []

  [threshold_kf]
    type = ParsedPostprocessor
    expression = 'if (max_err_kf <= th, 1, 0)'
    pp_names = 'max_err_kf'
    constant_names = 'th'
    constant_expressions = '2e-2'
  []
  [threshold_de]
    type = ParsedPostprocessor
    expression = 'if (max_err_de <= th, 1, 0)'
    pp_names = 'max_err_de'
    constant_names = 'th'
    constant_expressions = '2e-2'
  []
[]

[Outputs]
  csv = true
  hide = 'max_err_kf max_err_de'
  execute_on = 'TIMESTEP_END'
[]
