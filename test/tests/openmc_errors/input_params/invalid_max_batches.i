[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  temperature_blocks = '100'
  cell_level = 0
  tally_type = cell
  normalize_by_global_tally = false
  initial_properties = xml

  # there are only 10 inactive batches, so we should error
  max_batches = 8
  k_trigger = std_dev
  k_trigger_threshold = 1e-2
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
