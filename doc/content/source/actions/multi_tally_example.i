[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = mesh_in.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  particles = 10000
  inactive_batches = 100
  batches = 1000
  initial_properties = xml
  verbose = true
  cell_level = 0

  normalize_by_global_tally = true
  check_tally_sum = true

  power = 100.0

  source_rate_normalization = 'heating_local'

  [Tallies]
    [Mesh_Tally]
      type = MeshTally
      score = 'flux'
    []
    [Cell_Tally]
      type = CellTally
      score = 'heating_local'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = final
  exodus = true
[]
