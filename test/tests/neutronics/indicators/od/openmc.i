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
  [Indicators]
    [optical_depth_hmin]
      type = ElementOpticalDepthIndicator
      rxn_rate = 'total'
      h_type = 'min'
    []
    [optical_depth_hmax]
      type = ElementOpticalDepthIndicator
      rxn_rate = 'total'
      h_type = 'max'
    []
    [optical_depth_cuberoot]
      type = ElementOpticalDepthIndicator
      rxn_rate = 'total'
      h_type = 'cube_root'
    []
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0
  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Mesh]
      type = MeshTally
      score = 'kappa_fission flux total'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp cell_instance cell_id kappa_fission flux total'
[]
