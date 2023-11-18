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
  initial_properties = hdf5
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
[]
