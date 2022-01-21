# This input tests what happens if the OpenMC domain is fully enclosed in
# the MOOSE domain such that all OpenMC cells are coupled, but some MOOSE
# cells do not map anywhere in OpenMC. We should print a warning.

[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid]
    # Combine together to form four pebbles; this mesh would be the same as
    # whatever is used to solve for the solid phase. One of these pebbles
    # will be far outside the OpenMC domain, and will be unmapped
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
  [fluid]
    # Mesh of the fluid phase; this mesh would be the same as whatever is used to
    # solve for the fluid phase
    type = FileMeshGenerator
    file = stoplight.exo
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

  parallel_type = replicated
[]

[Problem]
  type = OpenMCCellAverageProblem
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
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
