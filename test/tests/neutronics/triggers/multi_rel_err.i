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
  normalize_by_global_tally = false

  source_rate_normalization = 'kappa_fission'

  max_batches = 200

  [Tallies]
    [Cell]
      type = CellTally
      block = '100'
      score = 'damage_energy kappa_fission'

      trigger = 'rel_err rel_err'
      trigger_threshold = '2e-2 2e-2'
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
[]

[Outputs]
  csv = true
[]
