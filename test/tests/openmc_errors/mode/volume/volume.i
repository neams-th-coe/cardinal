[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../../neutronics/meshes/sphere.e
  []
  [combine]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [block]
    type = SubdomainIDGenerator
    input = combine
    subdomain_id = '0'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '0'
  initial_properties = xml
  cell_level = 0
  tally_type = cell
  power = 100.0
  check_tally_sum = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
