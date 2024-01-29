[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []

  allow_renumbering = false
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true
  tally_type = mesh
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
