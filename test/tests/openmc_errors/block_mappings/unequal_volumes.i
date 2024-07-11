[Mesh]
  [sphere]
    # Mesh of a single pebble with outer radius of 1.5 (cm)
    type = FileMeshGenerator
    file = ../../neutronics/meshes/sphere.e
  []
  [sphere_shrunken]
    # Mesh of a single pebble with outer radius of 1.4 (cm) - this approximates
    # the effect of an OpenMC model have equal cell tally volumes but not mapping
    # to identical volumes in the [Mesh]
    type = TransformGenerator
    input = sphere
    transform = SCALE
    vector_value = '0.93 0.93 0.93'
  []
  [solid]
    type = CombinerGenerator
    inputs = 'sphere sphere_shrunken sphere'
    positions = '0 0 0
                 0 0 4
                 0 0 8'
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = solid
    subdomain_id = '100'
  []
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100'
    check_equal_mapped_tally_volumes = true
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 70.0
  temperature_blocks = '100'
  verbose = true
  cell_level = 0
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
