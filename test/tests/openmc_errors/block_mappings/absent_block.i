# This test checks that if we specify 'fluid_blocks', but no fluid maps to OpenMC,
# that we get an error.

[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '0 0 0'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
  [fluid]
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [fluid_ids]
    type = SubdomainIDGenerator
    input = fluid
    subdomain_id = '500'
  []
  [combine]
    type = CombinerGenerator
    inputs = 'solid_ids fluid_ids'
    positions = '0 0 0
                 100 0 0'
  []

  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 70.0
  solid_blocks = '100'
  fluid_blocks = '500'
  tally_blocks = '100'
  solid_cell_level = 0
  fluid_cell_level = 0
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
