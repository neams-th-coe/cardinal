[Mesh]
  [file]
    type = FileMeshGenerator
    file = bundle_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = file
    boundary = '${vessel_inner}'
    radius = '${fparse vessel_inner_diameter / 2.0}'
    layers = '${e_per_bl}'
    boundaries_to_rebuild = '${inlet} ${outlet} ${duct_outer} ${vessel_inner} ${vessel_inner_cut} ${duct_pad_outer}'
    geometry_type = cylinder

    curve_corners = true
    polygon_sides = 6
    polygon_size = ${fparse outer_flat_to_flat / sqrt(3.0)}
    polygon_layers = ${e_per_bl}
    polygon_layer_smoothing = ${cs}
    polygon_origins = ${polygon_origins}
    polygon_boundary = '${duct_outer}'
    corner_radius = ${fparse corner_radius}
  []
  [ordered]
    type = RenameBoundaryGenerator
    input = to_hex20
    old_boundary = '${duct_outer} ${vessel_inner} ${vessel_inner_cut} ${duct_pad_outer}'
    new_boundary = '3 4 5 6'
  []
  [scale]
    type = TransformGenerator
    input = ordered
    transform = SCALE
    vector_value = '${inv_Dh} ${inv_Dh} ${inv_Dh}'
  []
[]
