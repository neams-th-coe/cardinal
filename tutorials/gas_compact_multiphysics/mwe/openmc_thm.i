# This input file runs coupled OpenMC Monte Carlo transport, MOOSE heat
# conduction, and THM fluid flow and heat transfer.
# This input should be run with:
#
# cardinal-opt -i common_input.i openmc_thm.i

density_blocks = 'coolant'
temperature_blocks = 'graphite compacts compacts_trimmer_tri'
fuel_blocks = 'compacts compacts_trimmer_tri'

fuel_to_coolant_distance = 1.88e-2
compact_diameter = 1.27e-2
channel_diameter = 1.6e-2
height = 160e-2
n_layers = 50
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
    background_intervals = 3
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
[]

[ICs]
  [fluid_temp]
    type = FunctionIC
    variable = temp
    function = temp_ic
  []
[]

[Functions]
  [temp_ic]
    type = ParsedFunction
    expression = '800'
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  output = 'unrelaxed_tally_std_dev'
  check_equal_mapped_tally_volumes = true

  power = 75
  scaling = 100.0
  density_blocks = ${density_blocks}
  tally_blocks = ${fuel_blocks}
  tally_type = cell
  tally_name = heat_source
  cell_level = 1

  relaxation = robbins_monro

  temperature_variables = 'temp'
  temperature_blocks = '${temperature_blocks} ${density_blocks}'
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
