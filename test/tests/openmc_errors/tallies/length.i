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
  tally_type = cell
  tally_blocks = '100'
  solid_cell_level = 0
  initial_properties = xml
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
