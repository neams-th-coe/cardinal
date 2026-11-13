[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/hex_cube.e
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  power = 100.0
  source_rate_normalization = 'kappa_fission'

  [Tallies]
    [Mesh]
      type = MeshTally
      mesh_template = '../meshes/hex_cube.e'
      normalize_by_global_tally = false
    []
    [h3]
      type = MeshTally
      mesh_template = '../meshes/hex_cube.e'
      mesh_translations = '0 0 16'
      score = 'H3_production'
      normalize_by_global_tally = false
    []
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [total_h3]
    type = ElementIntegralVariablePostprocessor
    variable = H3_production
  []
[]

[Outputs]
  execute_on = final
  csv = true
[]
