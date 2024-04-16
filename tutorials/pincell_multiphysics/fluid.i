height = 0.5               # total height of the domain
pin_diameter = 0.97e-2     # pin outer diameter
pin_pitch = 1.28e-2        # pin pitch
num_layers = 25            # number of layers
fluid_id = 5               # block ID to assign to fluid (only used for bookkeeping)

bl_0 = 0.004855
bl_1 = 0.00486
bl_2 = 0.00488
bl_3 = 0.004934
bl_4 = 0.0050312
bl_5 = 0.0052061600000000005

[Mesh]
  [pin]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 4
    polygon_size = ${fparse pin_pitch / 2.0}
    num_sectors_per_side = '6 6 6 6'
    uniform_mesh_on_sides = true

    ring_radii = '${fparse pin_diameter / 2.0} ${bl_0} ${bl_1} ${bl_2} ${bl_3} ${bl_4} ${bl_5}'
    ring_intervals = '1 1 1 1 1 1 1'
    ring_block_ids = '${fparse fluid_id + 1} ${fluid_id} ${fluid_id} ${fluid_id} ${fluid_id} ${fluid_id} ${fluid_id}'

    background_block_ids = '${fparse fluid_id}'
    background_intervals = 6
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

  # We will name boundaries in the mesh for sidesets 2 - 7 using other MOOSE objects.
  # The reactor module does its own sideset naming, so we make sure those IDs are
  # free by deleting whatever the reactor module had.
  [delete_extra_surfaces]
    type = BoundaryDeletionGenerator
    input = rotate
    boundary_names = '2 3 4 5 6 7'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_extra_surfaces
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
    normal_tol = 1e-6
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
  [area_1] # should approximate 0.015236724369910496
    type = AreaPostprocessor
    boundary = '1'
  []
  [area_2] # should approximate 8.994188680593409e-05
    type = AreaPostprocessor
    boundary = '2'
  []
  [area_3] # should approximate 8.994188680593409e-05
    type = AreaPostprocessor
    boundary = '3'
  []
  [area_4] # should approximate 0.0064
    type = AreaPostprocessor
    boundary = '4'
  []
  [area_5] # should approximate 0.0064
    type = AreaPostprocessor
    boundary = '5'
  []
  [area_6] # should approximate 0.0064
    type = AreaPostprocessor
    boundary = '6'
  []
  [area_7] # should approximate 0.0064
    type = AreaPostprocessor
    boundary = '7'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
