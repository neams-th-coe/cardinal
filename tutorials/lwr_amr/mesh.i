#----------------------------------------------------------------------------------------
# Assembly geometrical information
#----------------------------------------------------------------------------------------
pitch        = 1.26
fuel_height  = 192.78
r_fuel       = 0.4095
r_gap_1      = 0.4180
r_clad_1     = 0.4750
r_guide      = 0.3400
r_guide_clad = 0.5400
#----------------------------------------------------------------------------------------

#----------------------------------------------------------------------------------------
# Meshing parameters
#----------------------------------------------------------------------------------------
NUM_SECTORS              = 2
FUEL_RADIAL_DIVISIONS    = 2
BACKGROUND_DIVISIONS     = 1
AXIAL_DIVISIONS          = 5
#----------------------------------------------------------------------------------------

[Mesh]
  [UO2_Pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS}'
    ring_radii = '${r_fuel} ${r_gap_1} ${r_clad_1}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '0 1 2 3'
    ring_block_names = 'uo2_center uo2 gap_1 zr_clad'
    background_block_ids = '17'
    background_block_names = 'water'
    background_intervals = ${BACKGROUND_DIVISIONS}

    flat_side_up = true
    quad_center_elements = false
    preserve_volumes = true

    create_outward_interface_boundaries = false
  []
  [MOX_4_3_Pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS}'
    ring_radii = '${r_fuel} ${r_gap_1} ${r_clad_1}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '4 5 2 3'
    ring_block_names = 'mox_43_center mox_43 gap_1 zr_clad'
    background_block_ids = '17'
    background_block_names = 'water'
    background_intervals = ${BACKGROUND_DIVISIONS}

    flat_side_up = true
    quad_center_elements = false
    preserve_volumes = true

    create_outward_interface_boundaries = false
  []
  [MOX_7_0_Pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS}'
    ring_radii = '${r_fuel} ${r_gap_1} ${r_clad_1}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '6 7 2 3'
    ring_block_names = 'mox_70_center mox_70 gap_1 zr_clad'
    background_block_ids = '17'
    background_block_names = 'water'
    background_intervals = ${BACKGROUND_DIVISIONS}

    flat_side_up = true
    quad_center_elements = false
    preserve_volumes = true

    create_outward_interface_boundaries = false
  []
  [MOX_8_7_Pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS}'
    ring_radii = '${r_fuel} ${r_gap_1} ${r_clad_1}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '8 9 2 3'
    ring_block_names = 'mox_87_center mox_87 gap_1 zr_clad'
    background_block_ids = '17'
    background_block_names = 'water'
    background_intervals = ${BACKGROUND_DIVISIONS}

    flat_side_up = true
    quad_center_elements = false
    preserve_volumes = true

    create_outward_interface_boundaries = false
  []
  [Guide_Tube_Pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS}'
    ring_radii = '${r_guide} ${r_guide_clad}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '10 11 12'
    ring_block_names = 'guide_center guide al_clad'
    background_block_ids = '17'
    background_block_names = 'water'
    background_intervals = ${BACKGROUND_DIVISIONS}

    flat_side_up = true
    quad_center_elements = false
    preserve_volumes = true

    create_outward_interface_boundaries = false
  []
  [Control_Rod_Pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS}'
    ring_radii = '${r_guide} ${r_guide_clad}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '13 14 12'
    ring_block_names = 'cr_center cr al_clad'
    background_block_ids = '17'
    background_block_names = 'water'
    background_intervals = ${BACKGROUND_DIVISIONS}

    flat_side_up = true
    quad_center_elements = false
    preserve_volumes = true

    create_outward_interface_boundaries = false
  []
  [Fission_Chamber_Pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    num_sectors_per_side = '${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS} ${NUM_SECTORS}'
    ring_radii = '${r_guide} ${r_guide_clad}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '15 16 12'
    ring_block_names = 'fission_center fission al_clad'
    background_block_ids = '17'
    background_block_names = 'water'
    background_intervals = ${BACKGROUND_DIVISIONS}

    flat_side_up = true
    quad_center_elements = false
    preserve_volumes = true

    create_outward_interface_boundaries = false
  []
  [UO2_Assembly]
    type = PatternedCartesianMeshGenerator
    inputs = 'UO2_Pin Guide_Tube_Pin Fission_Chamber_Pin'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 1 0 0 1 0 0 2 0 0 1 0 0 1 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'

    pattern_boundary = 'none'

    assign_type = 'cell'
    id_name = 'pin_id'
    generate_core_metadata = false
  []
  [UO2_Assembly_Rodded]
    type = PatternedCartesianMeshGenerator
    inputs = 'UO2_Pin Control_Rod_Pin Fission_Chamber_Pin'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 1 0 0 1 0 0 2 0 0 1 0 0 1 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 1 0 0 0 0 0 0 0 0 0 1 0 0 0;
               0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'

    pattern_boundary = 'none'

    assign_type = 'cell'
    id_name = 'pin_id'
    generate_core_metadata = false
  []
  [MOX_Assembly]
    type = PatternedCartesianMeshGenerator
    inputs = 'MOX_4_3_Pin MOX_7_0_Pin MOX_8_7_Pin Guide_Tube_Pin Fission_Chamber_Pin'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
               0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0;
               0 1 1 1 1 3 1 1 3 1 1 3 1 1 1 1 0;
               0 1 1 3 1 2 2 2 2 2 2 2 1 3 1 1 0;
               0 1 1 1 2 2 2 2 2 2 2 2 2 1 1 1 0;
               0 1 3 2 2 3 2 2 3 2 2 3 2 2 3 1 0;
               0 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 0;
               0 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 0;
               0 1 3 2 2 3 2 2 4 2 2 3 2 2 3 1 0;
               0 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 0;
               0 1 1 2 2 2 2 2 2 2 2 2 2 2 1 1 0;
               0 1 3 2 2 3 2 2 3 2 2 3 2 2 3 1 0;
               0 1 1 1 2 2 2 2 2 2 2 2 2 1 1 1 0;
               0 1 1 3 1 2 2 2 2 2 2 2 1 3 1 1 0;
               0 1 1 1 1 3 1 1 3 1 1 3 1 1 1 1 0;
               0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 0;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'

    pattern_boundary = 'none'

    assign_type = 'cell'
    id_name = 'pin_id'
    generate_core_metadata = false
  []
  [2D_Core]
    type = PatternedCartesianMeshGenerator
    inputs = 'UO2_Assembly UO2_Assembly_Rodded MOX_Assembly'
    pattern = '1 2;
               2 0'

    pattern_boundary = 'none'
    external_boundary_name = 'reflector_interface'

    assign_type = 'cell'
    id_name = 'pin_id'
    generate_core_metadata = true
  []
  [Delete_Blocks]
    type = BlockDeletionGenerator
    input = 2D_Core
    # Deleting the gap blocks to avoid erroneous mesh refinement.
    block = '2'
  []
  [To_Origin]
    type = TransformGenerator
    input = 'Delete_Blocks'
    transform = TRANSLATE_CENTER_ORIGIN
  []
  [3D_Core]
    type = AdvancedExtruderGenerator
    input = 'To_Origin'
    heights = '${fparse fuel_height}'
    num_layers = '${AXIAL_DIVISIONS}'
    direction = '0.0 0.0 1.0'
  []
[]
