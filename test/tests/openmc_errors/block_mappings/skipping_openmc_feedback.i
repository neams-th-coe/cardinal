# This input tests what happens if the MOOSE domain is fully enclosed in
# the OpenMC domain such that all MOOSE elements are coupled, but some OpenMC
# cells dont map anywhere in the MOOSE elements; we should print a warning.

[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [solid] # missing two of the pebbles actually present in the OpenMC problem
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
  verbose = true
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
