[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
  parallel_type = replicated
[]

[Adaptivity]
  [Indicators/stat_err]
    type = StatRelErrorIndicator
    score = 'kappa_fission'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  cell_level = 0

  power = 100.0

  [Tallies]
    [Mesh]
      type = MeshTally
      score = 'kappa_fission'
      name = 'mesh_kf'
      output = 'unrelaxed_tally_rel_error'

      normalize_by_global_tally = false
    []
    [Cell]
      type = CellTally
      score = 'kappa_fission'
      name = 'cell_kf'
      output = 'unrelaxed_tally_rel_error'

      normalize_by_global_tally = false
    []
    [Cell_2]
      type = CellTally
      score = 'heating'
      name = 'cell_2_heating'
      output = 'unrelaxed_tally_rel_error'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [max_rel]
    type = ElementExtremeValue
    variable = stat_err
    value_type = max
  []
[]

[Outputs]
  execute_on = timestep_end
  csv = true
[]
