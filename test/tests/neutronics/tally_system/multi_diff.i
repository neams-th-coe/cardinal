[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = ../heat_source/stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100 200'
  density_blocks = '200'
  cell_level = 0
  initial_properties = xml

  # The global tally check is disabled because we have a loosely fitting unstructured mesh tally.
  normalize_by_global_tally = false

  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Cell]
      type = CellTally
      score = 'kappa_fission'
      blocks = '100 200'
    []
    [Mesh]
      type = MeshTally
      score = 'flux'
      mesh_translations = '0 0 0
                           0 0 4
                           0 0 8'
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
