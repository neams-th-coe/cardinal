#----------------------------------------------------------------------------------------
# Assembly geometrical information
#----------------------------------------------------------------------------------------
pitch        = 1.26
fuel_height  = 192.78
r_fuel       = 0.4095
r_fuel_gap   = 0.4180
r_fuel_clad  = 0.4750
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
    ring_radii = '${r_fuel} ${r_fuel_gap} ${r_fuel_clad}'
    ring_intervals = '${FUEL_RADIAL_DIVISIONS} 1 1'
    polygon_size = ${fparse pitch / 2.0}

    ring_block_ids = '0 1 2 3'
    ring_block_names = 'uo2_center uo2 fuel_gap zr_clad'
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
  [Delete_Blocks]
    type = BlockDeletionGenerator
    input = UO2_Assembly
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
