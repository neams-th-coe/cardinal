Rf     = 0.5
pitch  = 1.4
R      = 0.55
height = 2.0
num_layers = 5

[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '2 2 2 2'
    ring_radii = '${fparse 0.75 * Rf} ${fparse 0.85 * Rf} ${fparse 0.95 * Rf} ${Rf} ${R}'
    ring_intervals = '1 1 1 1 2'
    ring_block_ids = '2 2 2 2 3'
    polygon_size = ${fparse pitch / 2.0}
    background_block_ids = '1'
    quad_center_elements = true
  []
  [delete_fluid]
    type = BlockDeletionGenerator
    input = pin
    block = '1'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_fluid
    heights = ${height}
    num_layers = ${num_layers}
    direction = '0 0 1'
  []
[]
