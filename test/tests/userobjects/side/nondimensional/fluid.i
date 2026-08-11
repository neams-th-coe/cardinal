flat_to_flat = 0.0129196
pin_radius = 0.003823
pin_pitch = ${fparse 1.19*pin_radius}
e_per_side = 2
h = 0.008
nl = 3

solid_id = 6
fluid_id = 7

[Mesh]
  [heated_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse flat_to_flat}
    num_sectors_per_side = '${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side}'
    uniform_mesh_on_sides = true

    ring_radii = '${pin_radius}'
    ring_intervals = '1'
    ring_block_ids = '${solid_id}'

    background_block_ids = '${fparse fluid_id}'
  []
  [assembly]
    type = PatternedHexMeshGenerator
    hexagon_size = ${fparse flat_to_flat}
    inputs = 'heated_pin'
    pattern = '0 0;
              0 0 0;
               0 0'
    pattern_boundary = hexagon
    background_block_id = ${fluid_id}
    background_intervals = 1
    rotate_angle = 60
  []

  # We will be using sidesets 2 and 3 to label the top and bottom,
  # boundary 1 for the pin surface, and boundary 4 for the duct surface,
  # so we clear them out here just to be extra sure those IDs
  # arent associated with some other part of the mesh
  [delete_surfaces]
    type = BoundaryDeletionGenerator
    input = assembly
    boundary_names = '1 2 3 4'
  []

  [pin_surface]
    type = SideSetsBetweenSubdomainsGenerator
    input = delete_surfaces
    primary_block = ${fluid_id}
    paired_block = ${solid_id}
    new_boundary = '1'
  []
  [delete_solid]
    type = BlockDeletionGenerator
    input = pin_surface
    block = '${solid_id}'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_solid
    direction = '0 0 1'
    num_layers = '${nl}'
    heights = '${h}'
    bottom_boundary = '2'
    top_boundary = '3'
  []
  [rename]
    type = RenameBoundaryGenerator
    input = extrude
    old_boundary = '10000'
    new_boundary = '4'
  []
[]
