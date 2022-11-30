garbage_2 = 501
garbage_3 = 502
garbage_4 = 503
garbage_5 = 504

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
    ring_block_ids = '${pin_id}'
    background_block_ids = '${pin_id}'

    # boundary layers on the inside, boundary layers on the outside, plus a flow gap and the duct.
    # These are listed from smallest to largest, and do not include the actual outer boundary
    # of the bundle unit cell (i.e. the flat-to-flat including the bundle-bundle gap)
    duct_sizes_style = apothem
    duct_sizes = '${fparse 0.9 * flat_to_flat / 2.0}
                  ${fparse flat_to_flat / 2.0}
                  ${fparse flat_to_flat / 2.0 + thickness}
                  ${obl_height}
                  ${oobl_height}'

    # layers inside duct (deleted later, so the value doesnt matter)
    # layers in duct (deleted later, so the value doesnt matter)
    # layers in the load pad region (boundary layers, plus background)
    # layers in the pad-ring gap region (boundary layers, plus background)
    duct_intervals = '1
                      1
                      ${di_lp} ${e_per_load_pad_span}
                      ${di_lp_gap} 1'

    duct_block_ids = '${fluid_id} ${duct_id} ${dbi_lp} ${gi} ${gap_id}'
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'gap'
    pattern = ${bundle_pattern}
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
  [rename]
    type = RenameBoundaryGenerator
    input = excore
    old_boundary = '1 2'
    new_boundary = '50 51'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = rename
    direction = '0 0 1'
    num_layers = '${nl1} ${bl_layers} ${bl_layers} ${nl2} ${bl_layers} ${bl_layers} ${nl3} ${bl_layers} ${bl_layers} ${nl4} ${bl_layers} ${bl_layers} ${nl5}'
    heights = '${h1} ${lower_bl} ${upper_bl} ${h2} ${lower_bl} ${upper_bl} ${h3} ${lower_bl} ${upper_bl} ${h4} ${lower_bl} ${upper_bl} ${h5}'
    bottom_boundary = ${inlet}
    top_boundary = ${outlet}

    subdomain_swaps = '${delete_duct};
                       ${s_swaps}
                       ${s_swaps_extra}
                       ${periph_id} ${garbage_id3} ${load_pad_id} ${garbage_id2} ${delete_duct};
                       ${s_swaps_extra}
                       ${s_swaps}
                       ${delete_duct};
                       ${s_swaps}
                       ${s_swaps_extra}
                       ${periph_id} ${garbage_id3} ${load_pad_id} ${garbage_id2} ${delete_duct};
                       ${s_swaps_extra}
                       ${do_nothing}
                       ${delete_duct}'
  []

  # The subdomain_swaps in AdvancedExtruderGenerator unfortunately only changes the subdomain IDs,
  # but the SideSetsBetweenSubdomainsGenerator wants subdomain NAMES. We introduce another
  # mesh generator just to get consistent names + IDs.
  [rename_subdomains]
    type = RenameBlockGenerator
    input = extrude
    old_block = '${garbage_id2} ${garbage_id3} ${garbage_id4}'
    new_block = '${garbage_2} ${garbage_3} ${garbage_4}'
  []
  [delete_pins]
    type = BlockDeletionGenerator
    input = rename_subdomains
    block = '${pin_id}'
  []
  [delete_duct]
    type = BlockDeletionGenerator
    input = delete_pins
    block = '${garbage_4}'
    new_boundary = '${duct_outer}'
  []
  [delete_pads]
    type = BlockDeletionGenerator
    input = delete_duct
    block = '${garbage_2}'
    new_boundary = '${duct_pad_outer}'
  []
  [delete_restraint_ring]
    type = BlockDeletionGenerator
    input = delete_pads
    block = '${garbage_3}'
    new_boundary = '${vessel_inner_cut}'
  []
  [rename_exterior]
    type = RenameBoundaryGenerator
    input = delete_restraint_ring
    old_boundary = '10000'
    new_boundary = '${vessel_inner}'
  []
  [delete_stuff]
    type = BlockDeletionGenerator
    input = rename_exterior
    block = '${fluid_id}'
  []

  second_order = true
[]
