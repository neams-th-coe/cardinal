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
[]

[Tallies]
  [Cell]
    type = CellTally
    tally_blocks = '100'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 70.0
  temperature_blocks = '100'
  cell_level = 0
  fixed_mesh = true
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Adaptivity]
  [Markers]
    [error_tol_marker]
      type = UniformMarker
      mark = refine
    []
  []
[]

[Outputs]
  exodus = true
[]
