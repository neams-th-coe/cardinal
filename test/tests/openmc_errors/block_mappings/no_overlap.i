# This input tests that an error should be produced if there is zero overlap
# between the MOOSE and OpenMC domains.

[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    type = CombinerGenerator
    inputs = sphere
    positions = '100 100 100'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 70.0
  temperature_blocks = '100'
  tally_type = cell
  tally_blocks = '100'
  cell_level = 0
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
