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
  [Markers/lh]
    type = ErrorFractionLookAheadMarker
    # Statistical error
    rel_error_refine = 0.2
    stat_error_indicator = 'stat_err'
    # Spatial error
    indicator = 'od'
    refine = 0.3
    coarsen = 0.0
  []
  [Indicators]
    [od]
      type = ElementOpticalDepthIndicator
      rxn_rate = 'fission'
      h_type = 'cube_root'
    []
    [stat_err]
      type = StatRelErrorIndicator
      score = 'kappa_fission'
    []
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  normalize_by_global_tally = false

  power = 100.0
  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Mesh_1]
      type = MeshTally
      score = 'kappa_fission'
      output = 'unrelaxed_tally_rel_error'
    []
    [Mesh_2]
      type = MeshTally
      score = 'flux fission'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'kappa_fission kappa_fission_rel_error fission flux od stat_err'
[]
