!include ../common.i

[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    ring_radii = '${fparse 1e-2*hole_diameter/2} ${fparse 1e-2*pellet_diameter/2} ${fparse 1e-2*inner_clad_diameter/2} ${fparse 1e-2*outer_clad_diameter/2}'
    ring_intervals = '1 5 1 1'
    num_sectors_per_side = '4 4 4 4 4 4'
    polygon_size = ${fparse 0.5 * pin_pitch * 1e-2}

    ring_block_ids = '0 1 0 2'
    ring_block_names = 'helium fuel helium clad'
    background_block_ids = '3'
    background_block_names = 'sodium'
    quad_center_elements = true
  []
  [assembly]
    type = PatternedHexMeshGenerator
    inputs = 'pin'
    pattern = '0 0 0 0 0;
              0 0 0 0 0 0;
             0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0;
              0 0 0 0 0 0;
               0 0 0 0 0'
    hexagon_size = ${fparse duct_inner_flat_to_flat * 1e-2/2}
    background_block_id = '3'
    background_block_name = 'sodium'
    rotate_angle = 60
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = assembly
    direction = '0 0 1'
    num_layers = '${n_layers}'
    heights = '${fparse height * 1e-2}'
  []
[]
