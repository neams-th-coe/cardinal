[Mesh]
  [file]
    type = FileMeshGenerator
    file = fluid_in.e
  []
  [to_hex20]
    type = NekMeshGenerator
    input = file
    boundaries_to_rebuild = '1 2 3 4'
    geometry_type = cylinder

    boundary = '1'
    radius = '${fparse pin_diameter / 2.0}'
    origins = '${pin_centers}'
    layers = '${e_per_bl}'

    # parameters related to applying radius of curvature to duct corners
    curve_corners = ${corners}
    polygon_sides = 6
    polygon_size = ${fparse flat_to_flat / sqrt(3.0)}
    corner_radius = ${duct_corner_radius_of_curvature}
    polygon_layers = ${fparse e_per_bl + e_per_assembly_background}
    polygon_layer_smoothing = ${corner_smoothing}
    polygon_boundary = '4'
    rotation_angle = 30.0
  []
  [scale]
    type = TransformGenerator
    input = to_hex20
    transform = scale
    vector_value = '${fparse 1.0 / hydraulic_diameter} ${fparse 1.0 / hydraulic_diameter} ${fparse 1.0 / hydraulic_diameter}'
  []
  [rotate]
    type = TransformGenerator
    input = scale
    transform = rotate
    vector_value = '30.0 0.0 0.0'
  []
[]

# The following content is adding postprocessor(s) to check sideset areas
# to have a regression test.
[Problem]
  type = FEProblem
  solve = false
[]

[Postprocessors]
  [area_pin] # should be slightly less than 1717.9180263134044 (when curving corners)
    type = AreaPostprocessor
    boundary = '1'
  []
  [area_bot] # should be slightly less than 16.09466649575005 (when curving corners)
    type = AreaPostprocessor
    boundary = '2'
  []
  [area_top] # should be slightly less than 16.09466649575005 (when curving corners)
    type = AreaPostprocessor
    boundary = '3'
  []
  [area_duct] # should be slightly less than 992.5486865696435 (when curving corners)
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
