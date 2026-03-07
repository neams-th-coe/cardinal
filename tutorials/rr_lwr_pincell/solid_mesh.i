!include common.i

[Mesh]
  [Pincell]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '4 4 4 4'
    ring_radii = '0.23642494 0.33435535 ${R_FUEL}
                  ${fparse R_FUEL + T_F_C_GAP}
                  ${fparse R_FUEL + T_F_C_GAP + T_ZR_CLAD}'
    ring_intervals = '1 1 1 1 1'
    polygon_size = ${fparse 0.5 * PITCH}

    ring_block_ids = '0 1 1
                      2
                      3'
    ring_block_names = 'uo2_tri uo2 uo2
                        gap
                        clad'
    background_block_ids = '4'
    background_block_names = 'water'
    background_intervals = 1

    flat_side_up = true
    quad_center_elements = false

    create_outward_interface_boundaries = false
  []
  [3D_Core]
    type = AdvancedExtruderGenerator
    input = 'Pincell'
    heights = '${HEIGHT}'
    num_layers = '${AXIAL_LAYERS}'
    direction = '0.0 0.0 1.0'
  []
  [To_Origin]
    type = TransformGenerator
    input = '3D_Core'
    transform = TRANSLATE_CENTER_ORIGIN
  []
  [Label_Fuel_Outer]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'To_Origin'
    primary_block = 'uo2'
    paired_block = 'gap'
    new_boundary = 'fuel_or'
  []
  [Label_Clad_Inner]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'Label_Fuel_Outer'
    primary_block = 'clad'
    paired_block = 'gap'
    new_boundary = 'clad_ir'
  []
  [Label_Clad_Outer]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'Label_Clad_Inner'
    primary_block = 'clad'
    paired_block = 'water'
    new_boundary = 'clad_or'
  []
  [Delete_Gap]
    type = BlockDeletionGenerator
    input = 'Label_Clad_Outer'
    block = 'gap'
  []
  [To_Meters]
    type = TransformGenerator
    input = 'Delete_Gap'
    transform = SCALE
    vector_value = '1e-2 1e-2 1e-2'
  []
[]
