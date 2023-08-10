[Mesh]
  [file]
    type = FileMeshGenerator
    file = plane_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = file
    boundary = '4'
    radius = ${fparse vessel_inner_diameter / 2.0}
    boundaries_to_rebuild = '1 2 3 4'
    layers = '${e_per_bl}'
    geometry_type = cylinder

    curve_corners = true
    polygon_sides = 6
    polygon_size = ${fparse outer_flat_to_flat / sqrt(3.0)}
    polygon_layer_smoothing = ${cs}
    polygon_layers = ${e_per_bl}
    polygon_origins = ${polygon_origins}
    polygon_boundary = '3'
    corner_radius = ${fparse corner_radius}
  []
  [scale]
    type = TransformGenerator
    input = to_hex20
    transform = scale
    vector_value = '${fparse 1.0 / hydraulic_diameter} ${fparse 1.0 / hydraulic_diameter} ${fparse 1.0 / hydraulic_diameter}'
  []
[]
