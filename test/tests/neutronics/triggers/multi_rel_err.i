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

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100'
    tally_score = 'damage_energy kappa_fission'

    tally_trigger = 'rel_err rel_err'
    tally_trigger_threshold = '2e-2 2e-2'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  batches = 20
  cell_level = 0
  normalize_by_global_tally = false

  max_batches = 200
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
