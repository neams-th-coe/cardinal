[Mesh]
  [mixed_elem_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    ring_radii = 4.0
    ring_intervals = 2
    polygon_size = 5.0
  []
  [delete_background]
    type = BlockDeletionGenerator
    input = mixed_elem_pin
    block = '3'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_background
    heights = '1'
    num_layers = '1'
    direction = '0 0 1'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  power = 100.0

  [Tallies]
    [Mesh]
      type = XDGMeshTally
    []
  []
[]

[Executioner]
  type = Steady
[]
