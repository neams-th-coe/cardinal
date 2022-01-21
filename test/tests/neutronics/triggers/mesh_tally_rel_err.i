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

  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 100.0
  solid_blocks = '100'
  solid_cell_level = 0
  tally_type = mesh
  mesh_template = '../meshes/sphere.e'
  mesh_translations = '0 0 0
                       0 0 4
                       0 0 8'
  normalize_by_global_tally = false
  initial_properties = xml

  tally_trigger = rel_err
  tally_trigger_threshold = 5e-1
  particles = 1000
  max_batches = 200
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [max_tally_err]
    type = FissionTallyRelativeError
  []
[]

[Outputs]
  csv = true
[]
