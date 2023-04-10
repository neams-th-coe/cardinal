# This input tests what happens if an OpenMC material maps to more than one phase

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
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = ../block_mappings/stoplight.exo
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '200'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  power = 70.0
  solid_blocks = '100'
  fluid_blocks = '200'
  tally_type = cell
  tally_blocks = '100'
  solid_cell_level = 0
  fluid_cell_level = 0
  initial_properties = xml
[]

[Executioner]
  type = Transient
[]
