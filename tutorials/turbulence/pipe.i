R = 0.5
nl = 8

[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '8 8 8 8'
    ring_radii = '${fparse 0.7 * R} ${fparse 0.8 * R} ${R}'
    ring_radial_biases = '1 1 0.8'
    ring_intervals = '1 1 ${nl}'
    ring_block_ids = '2 2 2'
    polygon_size = ${fparse 2 * R}
    background_block_ids = '1'
    quad_center_elements = true
    uniform_mesh_on_sides = true
    quad_element_type = QUAD9
  []
  [delete_background]
    type = BlockDeletionGenerator
    input = pin
    block = '1'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_background
    direction = '0 0 1'
    num_layers = 40
    heights = '10'
  []
  [delete_sides]
    type = BoundaryDeletionGenerator
    input = extrude
    boundary_names = '1 3'
  []
  [rename]
    type = RenameBoundaryGenerator
    input = delete_sides
    old_boundary = '5 6 7'
    new_boundary = '1 2 3'
  []
  [hex20]
    type = NekMeshGenerator
    input = rename
  []
[]
