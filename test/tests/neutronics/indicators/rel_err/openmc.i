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
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0
  normalize_by_global_tally = false

  power = 100.0
  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Mesh]
      type = MeshTally
      score = 'kappa_fission'
      output = 'unrelaxed_tally_rel_error'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp cell_instance cell_id kappa_fission kappa_fission_rel_error'
[]
