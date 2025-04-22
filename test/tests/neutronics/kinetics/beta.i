[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  calc_kinetics_params = false
  ifp_generations = 5
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [beta]
    type = BetaEffective
  []
[]

[Outputs]
  csv = true
[]
