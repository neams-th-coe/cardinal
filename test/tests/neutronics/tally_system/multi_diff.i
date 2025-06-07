[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  cell_level = 0
  initial_properties = xml

  # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
  normalize_by_global_tally = false

  source_rate_normalization = 'kappa_fission'

  # Missing some hits in the model (only tallying a single pebble instead of 3)
  check_tally_sum = false

  [Tallies]
    [Cell]
      type = CellTally
      score = 'kappa_fission'
      block = '100'
    []
    [Mesh]
      type = MeshTally
      score = 'flux'
      mesh_translations = '0 0 0'
      mesh_template = ../meshes/sphere.e
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
