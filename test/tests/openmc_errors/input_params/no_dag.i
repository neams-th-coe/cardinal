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
  solid_blocks = '100'
  solid_cell_level = 0
  tally_type = cell
  normalize_by_global_tally = false
  initial_properties = xml
  skinner = moab
[]

[UserObjects]
  [moab]
    type = MoabSkinner
    temperature = "temp"
    temperature_max = 1000.0
    n_temperature_bins = 5
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  csv = true
[]
