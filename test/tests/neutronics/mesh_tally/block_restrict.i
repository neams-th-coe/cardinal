[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid1]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []
  [sphereb]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid2]
    type = SubdomainIDGenerator
    input = sphereb
    subdomain_id = '200'
  []
  [spherec]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid3]
    type = SubdomainIDGenerator
    input = spherec
    subdomain_id = '300'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid1 solid2 solid3'
    positions_file = pebble_centers.txt
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100 200 300'
  initial_properties = xml
  verbose = true
  cell_level = 0
  normalize_by_global_tally = false

  power = 100.0

  [Tallies]
    [Mesh]
      type = MeshTally
      blocks = '200 300'
    []
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp  cell_instance cell_id'
[]
