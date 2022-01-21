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

  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0
  solid_blocks = '100'
  solid_cell_level = 0
  tally_type = cell
  normalize_by_global_tally = false
  initial_properties = xml

  # there are only 10 inactive batches, so we should error
  batches = 8
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
