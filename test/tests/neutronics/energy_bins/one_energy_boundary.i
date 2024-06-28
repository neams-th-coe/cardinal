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

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  normalize_by_global_tally = false

  cell_level = 0
  tally_blocks = '100'
  tally_type = cell
  power = 100.0

  energy_bin_boundaries = '0.0'
[]

[Executioner]
  type = Steady
[]
