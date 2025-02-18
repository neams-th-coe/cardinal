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
  [Delete_Blocks]
    type = BlockDeletionGenerator
    input = UO2_Pin
    # Deleting the gap blocks to avoid erroneous mesh refinement.
    block = '2'
  []
  [Left]
    type = TransformGenerator
    input = 'Delete_Blocks'
    transform = TRANSLATE
    vector_value = '${fparse -1 * pitch} ${fparse -1 * pitch} 0.0'
  []
  [3D_Core]
    type = AdvancedExtruderGenerator
    input = 'Left'
    heights = '${fparse fuel_height}'
    num_layers = '${AXIAL_DIVISIONS}'
    direction = '0.0 0.0 1.0'
  []
[]
