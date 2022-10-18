half_pitch = ${fparse bundle_pitch / 2.0}

[Mesh]
  [gap]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse bundle_pitch / 2.0}
    num_sectors_per_side = '${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side}'
    uniform_mesh_on_sides = true

    # dummy values related to the pin that will be deleted
    ring_radii = ${fparse 0.8 * flat_to_flat / 2.0}
    ring_intervals = '1'

    # assign dummy values to the other blocks so we know how to refer to them
    background_block_ids = '${fparse gap_id + 2}'
    ring_block_ids = '${fparse gap_id + 1}'

    duct_sizes = '${fparse flat_to_flat / 2.0 + 0.9 * thickness} ${fparse flat_to_flat / 2.0 + thickness} ${bl_height}'
    duct_block_ids = '${load_pad_id} ${pad_ids} ${gap_id}'
    duct_intervals = '1 ${duct_ids} 1'
    duct_sizes_style = apothem
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'gap'
    pattern = ${pattern}
    pattern_boundary = none
  []
  [excore]
    type = PeripheralRingMeshGenerator
    input = core
    peripheral_layer_num = ${e_per_peripheral}
    peripheral_ring_radius = ${fparse vessel_inner_diameter / 2.0}
    input_mesh_external_boundary = 10000
    peripheral_ring_block_id = ${periph_id}

    peripheral_outer_boundary_layer_bias = ${fparse 1.0 / growth_factor}
    peripheral_outer_boundary_layer_width = ${bl_outer_dx}
    peripheral_outer_boundary_layer_intervals = ${e_per_bl}
  []
  [inner_wall]
    type = SideSetsBetweenSubdomainsGenerator
    input = excore
    primary_block = '${gap_id}'
    paired_block = '${fparse load_pad_id}'
    new_boundary = 3
  []
  [delete_non_gap]
    type = BlockDeletionGenerator
    input = inner_wall
    block = '${fparse gap_id + 1} ${fparse gap_id + 2} ${load_pad_id}'
  []
  [delete_extra]
    type = BoundaryDeletionGenerator
    input = delete_non_gap
    boundary_names = '4 5'
  []

  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_extra
    direction = '0 0 1'
    num_layers = '${nl}'
    heights = '${h}'
    bottom_boundary = 1
    top_boundary = 2
  []
  [rename_exterior]
    type = RenameBoundaryGenerator
    input = extrude
    old_boundary = '10000'
    new_boundary = '4'
  []

  second_order = true
[]
