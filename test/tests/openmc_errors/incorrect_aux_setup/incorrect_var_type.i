[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0
                 0 0 4
                 0 0 8
                 9 9 9'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
[]

[AuxVariables]
  [cell_id]
  []
[]

[AuxKernels]
  [cell_id]
    type = CellIDAux
    variable = cell_id
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 70.0
  temperature_blocks = '100'
  tally_type = cell
  tally_blocks = '100'
  cell_level = 0
  initial_properties = xml
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
