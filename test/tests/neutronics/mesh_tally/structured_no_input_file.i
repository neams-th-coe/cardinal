[Mesh]
  [cylinder]
    type = FileMeshGenerator
    file = ../meshes/cylinder.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = cylinder
    subdomain_id = '200'
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '200'
  initial_properties = xml
  verbose = true
  cell_level = 0

  power = 100.0

  [Tallies]
    [StructuredMesh]
      type = StructuredMeshTally
      mesh_type = regular
      dimensions = 2
      lower_left = 0.0 0.0
      upper_right = 1.0 1.0
      nx = 8
      ny = 8
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
