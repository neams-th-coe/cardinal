[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../../meshes/sphere.e
  []
[]

[Problem]
  type = OpenMCCellAverageProblem
  verbose = true

  calc_kinetics_params = true
  ifp_generations = 5
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
