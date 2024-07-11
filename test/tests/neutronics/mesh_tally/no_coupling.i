[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
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
  verbose = true
  global_tally_estimator = collision
  check_tally_sum = false
  power = 100.0
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [power]
    type = ElementIntegralVariablePostprocessor
    variable = kappa_fission
  []
[]

[Outputs]
  csv = true
[]
