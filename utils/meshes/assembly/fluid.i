pin_radius = ${fparse pin_diameter / 2.0}
solid_id = 15

[Mesh]
  [heated_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse pin_pitch / 2.0}
    num_sectors_per_side = '${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side}'
    uniform_mesh_on_sides = true

    ring_radii = '${pin_radius} ${bl_radii}'
    ring_intervals = '1 ${fluid_elems}'
    ring_block_ids = '${solid_id} ${fluid_ids}'

    background_block_ids = '${fparse fluid_id}'
    background_intervals = ${e_per_pin_background}
  []
  [assembly]
    type = PatternedHexMeshGenerator
    hexagon_size = ${fparse flat_to_flat / 2.0}
    inputs = 'heated_pin'
    pattern = ${pattern}
    pattern_boundary = hexagon
    background_block_id = ${fluid_id}
    background_intervals = ${e_per_assembly_background}

    duct_sizes_style = apothem
    duct_sizes = '${duct_radii}'
    duct_intervals = '${fluid_elems}'
    duct_block_ids = '${fluid_ids}'
  []
  [pin_surface]
    type = SideSetsBetweenSubdomainsGenerator
    input = assembly
    primary_block = ${fluid_id}
    paired_block = ${solid_id}
    new_boundary = '8'
  []
  [delete_solid]
    type = BlockDeletionGenerator
    input = pin_surface
    block = '${solid_id}'
  []
  [delete_extra_pin_surfaces]
    type = RenameBoundaryGenerator
    input = delete_solid
    old_boundary = '2 3'
    new_boundary = '44 45'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_extra_pin_surfaces
    direction = '0 0 1'
    num_layers = '${nl}'
    heights = '${h}'
    bottom_boundary = '2'
    top_boundary = '3'
  []
  [rename_pin_surface]
    type = RenameBoundaryGenerator
    input = extrude
    old_boundary = '4'
    new_boundary = '44'
  []
  [rename_more]
    type = RenameBoundaryGenerator
    input = rename_pin_surface
    old_boundary = '10000'
    new_boundary = '4'
  []

  second_order = true
[]
