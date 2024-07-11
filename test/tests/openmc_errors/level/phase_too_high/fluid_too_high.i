[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../../../neutronics/meshes/sphere.e
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
    file = ../../../neutronics/heat_source/stoplight.exo
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

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100 200'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 70.0
  temperature_blocks = '100 200'
  density_blocks = '200'
  verbose = true

  # skip the data transfer of temperature into OpenMC for the first time step
  # so that we can just use the ICs set in OpenMCs XML files
  initial_properties = xml

  # For this setup, the fluid cells only exist on coordinate level 0, even though
  # the highest coordinate level across the entire problem is 1
  cell_level = 1
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
