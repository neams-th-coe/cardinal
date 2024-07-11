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
    tally_trigger = rel_err
    tally_trigger_threshold = 2e-2
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  cell_level = 0

  normalize_by_global_tally = false

  max_batches = 200
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_tally_err]
    type = TallyRelativeError
  []
[]

[Outputs]
  csv = true
[]
