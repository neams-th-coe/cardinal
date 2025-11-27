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
  power = 100.0

  [Tallies]
    [Mesh]
      type = MeshTally
      check_tally_sum = false
    []
  []
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
