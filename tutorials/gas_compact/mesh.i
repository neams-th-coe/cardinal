fuel_to_coolant_distance = 1.623e-2
compact_diameter = 1.27e-2
channel_diameter = 1.6e-2
height = 160e-2
n_layers = 30
ns = 8

[GlobalParams]
  quad_center_elements = true
[]

[Mesh]
  [fuel_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse 0.8 * compact_diameter / 2.0} ${fparse compact_diameter / 2.0}'
    ring_intervals = '1 1'
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    ring_block_ids = '2 2'
    ring_block_names = 'compacts compacts'
    background_block_names = 'graphite'
    background_intervals = 4
  []
  [coolant_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse fuel_to_coolant_distance / 2.0}
    ring_radii = '${fparse channel_diameter / 2.0}'
    ring_intervals = '2'
    num_sectors_per_side = '${ns} ${ns} ${ns} ${ns} ${ns} ${ns}'
    ring_block_ids = '101 101'
    ring_block_names = 'coolant coolant'
    background_block_names = 'graphite'
    interface_boundary_id_shift = 100
    background_intervals = 1
  []
  [bundle]
    type = PatternedHexMeshGenerator
    inputs = 'fuel_pin coolant_pin'
    hexagon_size = ${fparse 2.0 * fuel_to_coolant_distance}
    pattern = '0 0;
              0 1 0;
               0 0'
  []
  [trim]
    type = HexagonMeshTrimmer
    input = bundle
    trim_peripheral_region = '1 1 1 1 1 1'
    peripheral_trimming_section_boundary = peripheral_section
  []
  [rotate]
    type = TransformGenerator
    input = trim
    transform = rotate
    vector_value = '30.0 0.0 0.0'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = rotate
    heights = ${height}
    num_layers = ${n_layers}
    direction = '0 0 1'
  []
  [fluid_solid_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = extrude
    primary_block = 'graphite'
    paired_block = 'coolant'
    new_boundary = 'fluid_solid_interface'
  []
  [delete_coolant]
    type = BlockDeletionGenerator
    input = fluid_solid_interface
    block = 'coolant'
  []
[]

# The following content is adding postprocessor(s) to check sideset areas.
# The reactor module is unfortunately quite brittle in its assignment of sideset
# IDs, so we want to be extra sure that any changes to sideset numbering are detected
# in our test suite.
[Problem]
  type = FEProblem
  solve = false
[]

[Postprocessors]
  [area_walls] # should approximate 0.08042477193189872
    type = AreaPostprocessor
    boundary = 'fluid_solid_interface'
  []
  [volume_fuel] # should approximate 0.0004053659832779982
    type = VolumePostprocessor
    block = 'compacts compacts_trimmer_tri'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
