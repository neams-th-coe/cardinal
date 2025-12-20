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

  source_rate_normalization = 'kappa_fission'

  particles = 100
  max_batches = 200

  [Tallies]
    [Mesh_1]
      type = MeshTally
      score = kappa_fission
      trigger = rel_err
      trigger_threshold = 5e-1
      mesh_translations = '0 0 0'
      mesh_template = ../meshes/sphere.e

      # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
      normalize_by_global_tally = false
    []
    [Mesh_2]
      type = MeshTally
      score = flux
      trigger = rel_err
      trigger_threshold = 5e-1
      mesh_translations = '0 0 0'
      mesh_template = ../meshes/sphere.e

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
[]
