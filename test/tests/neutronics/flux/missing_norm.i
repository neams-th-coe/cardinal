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
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 1e4
  temperature_blocks = '100'
  tally_blocks = '100'
  cell_level = 0
  tally_type = cell
  initial_properties = xml

  tally_score = 'heating flux H3_production'
[]

[Executioner]
  type = Steady
[]
