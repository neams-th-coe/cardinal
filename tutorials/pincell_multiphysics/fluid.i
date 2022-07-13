height = 0.5               # total height of the domain
pin_diameter = 0.97e-2     # pin outer diameter
pin_pitch = 1.28e-2        # pin pitch
num_layers = 25            # number of layers
fluid_id = 5               # block ID to assign to fluid (only used for bookkeeping)

bl_height0 = 0.00488
bl_height1 = 0.004934
bl_height2 = 0.0050312
bl_height3 = 0.0052061600000000005

[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    polygon_size = ${fparse pin_pitch / 2.0}
    num_sectors_per_side = '6 6 6 6'
    uniform_mesh_on_sides = true

    # 3 boundary layers
    ring_radii = '${fparse pin_diameter / 2.0} ${bl_height0} ${bl_height1} ${bl_height2}'
    ring_intervals = '1 1 1 1'
    ring_block_ids = '${fparse fluid_id + 1} ${fluid_id} ${fluid_id} ${fluid_id}'

    background_block_ids = '${fparse fluid_id}'
    background_intervals = 3
  []
  [pin_surface]
    type = SideSetsBetweenSubdomainsGenerator
    input = pin
    primary_block = ${fluid_id}
    paired_block = ${fparse fluid_id + 1}
    new_boundary = '1'
  []
  [delete_solid]
    type = BlockDeletionGenerator
    input = pin_surface
    block = ${fparse fluid_id + 1}
  []
  [rotate]
    type = TransformGenerator
    input = delete_solid
    transform = rotate
    vector_value = '45.0 0.0 0.0'
  []
  [delete_extra_pin_surfaces] # this just makes sure boundary 2 and 3 are free for 'extrude'
    type = RenameBoundaryGenerator
    input = rotate
    old_boundary = '2 3'
    new_boundary = '45 46'
  []
  [rename_pin_surface] # this just makes sure boundary 4 is free for 'top_boundary'
    type = RenameBoundaryGenerator
    input = delete_extra_pin_surfaces
    old_boundary = '4'
    new_boundary = '44'
  []
  [extrude]
    type = FancyExtruderGenerator
    input = rename_pin_surface
    direction = '0 0 1'
    num_layers = '${num_layers}'
    heights = '${height}'
    bottom_boundary = '2'
    top_boundary = '3'
  []
  [lateral_boundaries]
    type = SideSetsFromNormalsGenerator
    input = extrude
    new_boundary =  '4 5 6 7'
    normals = '1.0  0.0 0.0
              -1.0  0.0 0.0
               0.0  1.0 0.0
               0.0 -1.0 0.0'
  []

  second_order = true
[]
