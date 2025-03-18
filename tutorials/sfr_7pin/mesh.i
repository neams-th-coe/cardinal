[Mesh]

# --- DUCT --- #

  # Make the duct mesh in 2-D; we first create a "solid" hexagon with two blocks
  # so that we can set the inner wall sideset as the boundary between these two blocks,
  # and then delete an inner block to get just the duct walls
  [duct]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse bundle_pitch / 2.0 + duct_thickness}
    num_sectors_per_side = '14 14 14 14 14 14'
    uniform_mesh_on_sides = true

    duct_sizes = '${fparse bundle_pitch / 2.0}'
    duct_block_ids = '10'
    duct_intervals = '4'
    duct_sizes_style = apothem
  []
  [inner_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = duct
    new_boundary = '10'
    primary_block = '10'
    paired_block = '1'
  []
  [delete_inside_block]
    type = BlockDeletionGenerator
    input = inner_sideset
    block = '1'
  []

  # rotate the duct by 30 degrees, then add names for sidesets. Delete sideset 1 because
  # we will name some other part of our mesh with sideset 1.
  [rotate]
    type = TransformGenerator
    input = delete_inside_block
    transform = rotate
    vector_value = '30.0 0.0 0.0'
  []
  [rename_sideset_names]
    type = RenameBoundaryGenerator
    input = rotate
    old_boundary = '10000 10'
    new_boundary = 'duct_outer duct_inner'
  []
  [delete_extraneous]
    type = BoundaryDeletionGenerator
    input = rename_sideset_names
    boundary_names = '1'
  []

# --- CLAD --- #

  [clad]
    type = AnnularMeshGenerator
    nr = 3
    nt = 20
    rmin = ${fparse d_pin / 2.0 - t_clad}
    rmax = ${fparse d_pin / 2.0}
    quad_subdomain_id = 1
    tri_subdomain_id = 0
  []
  [rename_clad] # this renames some sidesets on the clad to avoid name clashes
    type = RenameBoundaryGenerator
    input = clad
    old_boundary = '1 0' # outer surface, inner surface
    new_boundary = '5 4'
  []
  [rename_clad_names] # this renames some names on the clad to avoid name clashes
    type = RenameBoundaryGenerator
    input = rename_clad
    old_boundary = '5 4' # outer surface, inner surface
    new_boundary = 'clad_outer clad_inner'
  []

# --- FUEL --- #

  [fuel]
    type = AnnularMeshGenerator
    nr = 10
    nt = 20
    rmin = 0
    rmax = ${fparse d_pellet / 2.0}
    quad_subdomain_id = 2
    tri_subdomain_id = 3
    growth_r = -1.2
  []

# --- COMBINE --- #

  [combine] # this combines the fuel and clad together to make one pin
    type = CombinerGenerator
    inputs = 'rename_clad_names fuel'
  []
  [repeat] # this repeats the pincell 7 times to get the 7 pins, and adds the duct
    type = CombinerGenerator
    inputs = 'combine combine combine combine combine combine combine delete_extraneous'
    positions = '+0.00000000 +0.00000000 +0.00000000
                 +0.00452000 +0.00782887 +0.00000000
                 -0.00452000 +0.00782887 +0.00000000
                 -0.00904000 +0.00000000 +0.00000000
                 -0.00452000 -0.00782887 +0.00000000
                 +0.00452000 -0.00782887 +0.00000000
                 +0.00904000 +0.00000000 +0.00000000
                 +0.0        +0.0        +0.0'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = repeat
    direction = '0 0 1'
    num_layers = '40'
    heights = '${L}'
  []
[]
