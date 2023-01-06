pin_radius = ${fparse pin_diameter / 2.0}
pellet_radius = ${fparse pellet_diameter / 2.0}

[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse pin_pitch / 2.0}
    num_sectors_per_side = '${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side}'
    uniform_mesh_on_sides = true
    quad_center_elements = true

    ring_radii = '${pellet_radius} ${pin_radius}'
    ring_intervals = '${e_per_pellet} ${e_per_clad}'
    ring_block_ids = '${fuel_id} ${fuel_id} ${clad_id}'

    background_block_ids = '${fluid_id}'
    background_intervals = 1
  []
  [assembly]
    type = PatternedHexMeshGenerator
    hexagon_size = ${fparse flat_to_flat / 2.0}
    inputs = 'pin'
    pattern = ${pattern}
    pattern_boundary = hexagon
    background_block_id = ${fluid_id}
    background_intervals = 1
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'assembly'
    pattern = ${pattern}
    pattern_boundary = none
    generate_core_metadata = true
  []
  [delete_fluid]
    type = BlockDeletionGenerator
    input = core
    block = '${fluid_id}'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_fluid
    direction = '0 0 1'
    num_layers = '${nl}'
    heights = '${h}'
    bottom_boundary = 4
    top_boundary = 5
  []
  [delete_extraneous]
    type = BoundaryDeletionGenerator
    input = extrude
    boundary_names = '1 2'
  []
[]
