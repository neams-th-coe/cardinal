[Mesh]
  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0

  [Tallies]
    [StructuredMesh]
      type = StructuredMeshTally
      mesh_type = rectilinear
      dimensions = 2
      lower_left = 0.0 0.0
      upper_right = 1.0 1.0
      nx = 10
      ny = 10
      x_grid = 0.0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
      y_grid = 0.0 0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5
    []
  []

[Executioner]
  type = Steady
[]

[Postprocessors]
  [heat_source]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]

[Outputs]
  execute_on = final
  exodus = true
  hide = 'temp cell_instance cell_id'
[]