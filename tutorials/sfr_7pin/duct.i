L = 20.32e-2                                  # height (m)
bundle_pitch = 0.02625                        # flat-to-flat distance inside the duct (m)
duct_thickness = 0.004                        # duct thickness (m)

[Mesh]
  [duct]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    polygon_size = ${fparse bundle_pitch / 2.0 + duct_thickness}
    num_sectors_per_side = '10 10 10 10 10 10'
    uniform_mesh_on_sides = true

    duct_sizes = '${fparse bundle_pitch / 2.0}'
    duct_block_ids = '10'
    duct_intervals = '4'
    duct_sizes_style = apothem
  []
  [rotate]
    type = TransformGenerator
    input = duct
    transform = rotate
    vector_value = '30.0 0.0 0.0'
  []
  [inner_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = rotate
    new_boundary = '10'
    primary_block = '10'
    paired_block = '1'
  []
  [delete_inside_block]
    type = BlockDeletionGenerator
    input = inner_sideset
    block = '1'
  []
  [rename_sideset_ids]
    type = RenameBoundaryGenerator
    input = delete_inside_block
    old_boundary = '10000'
    new_boundary = '20'
  []
  [rename_sideset_names]
    type = RenameBoundaryGenerator
    input = rename_sideset_ids
    old_boundary = '20 10'
    new_boundary = 'duct_outer duct_inner'
  []
  [delete_extraneous]
    type = BoundaryDeletionGenerator
    input = rename_sideset_names
    boundary_names = '1'
  []
  [extrude]
    type = AdvancedExtruderGenerator
    input = delete_extraneous
    heights = '${L}'
    num_layers = '40'
    direction = '0 0 1'
  []
[]
