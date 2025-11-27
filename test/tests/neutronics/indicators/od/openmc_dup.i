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

[Adaptivity/Indicators/optical_depth_cuberoot]
  type = ElementOpticalDepthIndicator
  rxn_rate = 'total'
  h_type = 'cube_root'
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0
  source_rate_normalization = 'kappa_fission'
  normalization_tally = 'Cell'

  [Tallies]
    [Mesh]
      type = MeshTally
      score = 'kappa_fission flux total'
      name = 'mesh_kf mesh_flux mesh_total'

      normalize_by_global_tally = false
    []
    [Cell]
      type = CellTally
      score = 'kappa_fission flux total'
      name = 'cell_kf cell_flux cell_total'

      normalize_by_global_tally = false
    []
    [Cell_2]
      type = CellTally
      score = 'heating'
      name = 'cell_2_heating'

      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [max_od]
    type = ElementExtremeValue
    variable = optical_depth_cuberoot
    value_type = max
  []
[]

[Outputs]
  execute_on = timestep_end
  csv = true
[]
