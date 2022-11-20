n_layers = 1

channel_diameter = 0.016                 # diameter of the coolant channels (m)
compact_diameter = 0.0127                # diameter of fuel compacts (m)
fuel_to_coolant_distance = 0.0188        # distance between center of fuel compact and coolant channel (m)
bundle_flat_to_flat = 0.28               # bundle flat-to-flat distance (m)
bundle_gap_width = 4e-3                  # gap width between bundles (m)
height = 6.343                           # height of the full core (m)

[Mesh]
  [fuel_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse compact_diameter / 2.0}'
    ring_intervals = '1'
    num_sectors_per_side = '4 4 4 4 4 4'
    ring_block_ids = '2'
    ring_block_names = 'compacts'
    background_block_ids = '1'
    background_block_names = 'graphite'
    background_intervals = 1
  []
  [coolant_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse channel_diameter / 2.0}'
    ring_intervals = '1'
    num_sectors_per_side = '4 4 4 4 4 4'
    ring_block_ids = '101'
    ring_block_names = 'coolant'
    background_block_ids = '1'
    background_block_names = 'graphite'
    interface_boundary_id_shift = 100
    background_intervals = 1
  []
  [graphite_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse compact_diameter / 2.0}'
    ring_intervals = '1'
    num_sectors_per_side = '4 4 4 4 4 4'
    ring_block_ids = '1'
    ring_block_names = 'graphite'
    background_block_ids = '1'
    background_block_names = 'graphite'
    quad_center_elements = true
  []
  [bundle]
    type = PatternedHexMeshGenerator
    inputs = 'fuel_pin coolant_pin graphite_pin'
    hexagon_size = ${fparse bundle_flat_to_flat / 2.0 + bundle_gap_width / 2.0}
    pattern = '0 1 0 0 1 0 0 1 0;
              1 0 0 1 0 0 1 0 0 1;
             0 0 1 0 0 1 0 0 1 0 0;
            0 1 0 0 1 0 0 1 0 0 1 0;
           1 0 0 1 0 0 1 0 0 1 0 0 1;
          0 0 1 0 0 1 0 0 1 0 0 1 0 0;
         0 1 0 0 1 0 0 1 0 0 1 0 0 1 0;
        1 0 0 1 0 0 1 2 2 1 0 0 1 0 0 1;
       0 0 1 0 0 1 0 2 2 2 0 1 0 0 1 0 0;
        1 0 0 1 0 0 1 2 2 1 0 0 1 0 0 1;
         0 1 0 0 1 0 0 1 0 0 1 0 0 1 0;
          0 0 1 0 0 1 0 0 1 0 0 1 0 0;
           1 0 0 1 0 0 1 0 0 1 0 0 1;
            0 1 0 0 1 0 0 1 0 0 1 0;
             0 0 1 0 0 1 0 0 1 0 0;
              1 0 0 1 0 0 1 0 0 1;
               0 1 0 0 1 0 0 1 0'
    rotate_angle = 0

    background_block_id = '1'
    background_block_names = 'graphite'
    background_intervals = 1
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = bundle
    heights = ${height}
    num_layers = ${n_layers}
    direction = '0 0 1'
  []
  [delete_coolant]
    type = BlockDeletionGenerator
    input = extrude
    block = '101'
  []

  construct_side_list_from_node_list = true
[]
