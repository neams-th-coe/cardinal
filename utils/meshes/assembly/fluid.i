pin_radius = ${fparse pin_diameter / 2.0}
solid_id = 15

[Mesh]
  [heated_pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse pin_pitch / 2.0}
    num_sectors_per_side = '${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side} ${e_per_side}'
    uniform_mesh_on_sides = true

    ring_radii = '${pin_radius} ${bl_radii}'
    ring_intervals = '1 ${fluid_elems}'
    ring_block_ids = '${solid_id} ${fluid_ids}'

    background_block_ids = '${fparse fluid_id}'
    background_intervals = ${e_per_pin_background}
  []
  [assembly]
    type = PatternedHexMeshGenerator
    hexagon_size = ${fparse flat_to_flat / 2.0}
    inputs = 'heated_pin'
    pattern = ${pattern}
    pattern_boundary = hexagon
    background_block_id = ${fluid_id}
    background_intervals = ${e_per_assembly_background}

    duct_sizes_style = apothem
    duct_sizes = '${duct_radii}'
    duct_intervals = '${fluid_elems}'
    duct_block_ids = '${fluid_ids}'
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

  second_order = true
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
  [area_pin] # should match 0.05980442663288348
    type = AreaPostprocessor
    boundary = '1'
  []
  [area_bot] # should match 0.0005602900062067424
    type = AreaPostprocessor
    boundary = '2'
  []
  [area_top] # should match 0.0005602900062067424
    type = AreaPostprocessor
    boundary = '3'
  []
  [area_duct] # should match 0.034552757579999996
    type = AreaPostprocessor
    boundary = '4'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  exodus = true
[]
