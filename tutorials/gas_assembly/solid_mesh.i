n_layers = 100    # number of axial extrusion layers; for the converged case,
                  # we set this to 300 to get a finer mesh

[GlobalParams]
  quad_center_elements = true
[]

[Mesh]
  [fuel_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse 0.8 * compact_diameter / 2.0} ${fparse compact_diameter / 2.0}'
    ring_intervals = '1 1'
    num_sectors_per_side = '4 4 4 4 4 4'
    ring_block_ids = '2 2'
    ring_block_names = 'compacts compacts'
    background_block_ids = '1'
    background_block_names = 'graphite'
    background_intervals = 2
  []
  [coolant_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse channel_diameter / 2.0}'
    ring_intervals = '2'
    num_sectors_per_side = '4 4 4 4 4 4'
    ring_block_ids = '101 101'
    ring_block_names = 'coolant coolant'
    background_block_ids = '1'
    background_block_names = 'graphite'
    interface_boundary_id_shift = 100
    create_inward_interface_boundaries = true
    background_intervals = 2
  []
  [poison_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse compact_diameter / 2.0}'
    ring_intervals = '2'
    num_sectors_per_side = '4 4 4 4 4 4'
    ring_block_ids = '4 4'
    ring_block_names = 'poison poison'
    background_block_ids = '1'
    background_block_names = 'graphite'
    background_intervals = 2
  []
  [graphite_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse compact_diameter / 2.0}'
    ring_intervals = '2'
    num_sectors_per_side = '4 4 4 4 4 4'
    ring_block_ids = '1 1'
    ring_block_names = 'graphite graphite'
    background_block_ids = '1'
    background_block_names = 'graphite'
  []
  [bundle]
    type = PatternedHexMeshGenerator
    inputs = 'fuel_pin coolant_pin poison_pin graphite_pin'
    hexagon_size = ${fparse bundle_flat_to_flat / 2.0 + bundle_gap_width / 2.0}
    pattern = '2 0 1 0 0 1 0 0 1 0 2;
              0 1 0 0 1 0 0 1 0 0 1 0;
             1 0 0 1 0 0 1 0 0 1 0 0 1;
            0 0 1 0 0 1 0 0 1 0 0 1 0 0;
           0 1 0 0 1 0 0 1 0 0 1 0 0 1 0;
          1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1;
         0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0;
        0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0;
       1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1;
      0 0 1 0 0 1 0 0 1 3 3 1 0 0 1 0 0 1 0 0;
     2 1 0 0 1 0 0 1 0 3 3 3 0 1 0 0 1 0 0 1 2;
      0 0 1 0 0 1 0 0 1 3 3 1 0 0 1 0 0 1 0 0;
       1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1;
        0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0;
         0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0;
          1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1;
           0 1 0 0 1 0 0 1 0 0 1 0 0 1 0;
            0 0 1 0 0 1 0 0 1 0 0 1 0 0;
             1 0 0 1 0 0 1 0 0 1 0 0 1;
              0 1 0 0 1 0 0 1 0 0 1 0;
               2 0 1 0 0 1 0 0 1 0 2'
    rotate_angle = 0

    background_block_id = '1'
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
  [rename_coolant_sideset]
    type = RenameBoundaryGenerator
    input = delete_coolant
    old_boundary = 102
    new_boundary = 'fluid_solid_interface'
  []

  construct_side_list_from_node_list = true
[]

# The following content is adding postprocessor(s) to check sideset areas.
# The reactor module is unfortunately quite brittle in its assignment of sideset
# IDs, so we want to be extra sure that any changes to sideset numbering are detected
# in our test suite.
[Problem]
  type = FEProblem
  solve = false
[]

[Postprocessors]
  [area_walls]
    type = AreaPostprocessor
    boundary = 'fluid_solid_interface'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  exodus = true
[]
