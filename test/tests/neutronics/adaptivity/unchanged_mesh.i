[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [solid]
    type = CombinerGenerator
    inputs = solid_ids
    positions = '0 0 0'
    avoid_merging_subdomains = true
  []

  allow_renumbering = false
[]

[Adaptivity]
  steps = 1
  marker = uniform

  [Markers/uniform]
    type = UniformMarker
    mark = DO_NOTHING
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  [Tallies]
    [Mesh]
      type = MeshTally
      score = kappa_fission

      # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
  hide = 'temp cell_instance'
[]
