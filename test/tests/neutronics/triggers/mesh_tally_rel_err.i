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

  allow_renumbering = false
[]

[Tallies]
  [Mesh]
    type = MeshTally
    mesh_template = '../meshes/sphere.e'
    mesh_translations = '0 0 0
                         0 0 4
                         0 0 8'
    tally_trigger = rel_err
    tally_trigger_threshold = 5e-1
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 100.0
  normalize_by_global_tally = false

  particles = 1000
  max_batches = 200
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_tally_err]
    type = TallyRelativeError
  []
[]

[Outputs]
  csv = true
[]
