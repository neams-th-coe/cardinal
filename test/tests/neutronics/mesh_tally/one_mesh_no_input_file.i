[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
  [solid_ids]
    type = SubdomainIDGenerator
    input = sphere
    subdomain_id = '100'
  []

  allow_renumbering = false
[]

[Tallies]
  [Mesh]
    type = MeshTally
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  temperature_blocks = '100'
  initial_properties = xml
  verbose = true
  cell_level = 0
  normalize_by_global_tally = false

  power = 100.0
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
