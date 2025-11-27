[Mesh]
  [file]
    type = FileMeshGenerator
    file = mesh_in.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  particles = 10000
  inactive_batches = 50
  batches = 1000

  power = ${fparse 3000e6 / 273 * 4}

  assume_separate_tallies = true
  skip_statepoint = true

  [Tallies]
    [heat_source]
      type = MeshTally
      score = 'kappa_fission'
      name = 'heat_source'
      output = 'unrelaxed_tally_std_dev'
      block = 'uo2_center uo2'

      normalize_by_global_tally = false
    []
  []
[]

[Postprocessors]
  [max_rel_err]
    type = TallyRelativeError
    value_type = max
    tally_score = kappa_fission
  []
  [min_rel_err]
    type = TallyRelativeError
    value_type = min
    tally_score = kappa_fission
  []
  [avg_rel_err]
    type = TallyRelativeError
    value_type = average
    tally_score = kappa_fission
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  csv = true
[]
