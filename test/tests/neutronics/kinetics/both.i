[Mesh]
  [sphere]
    type = FileMeshGenerator
    file = ../meshes/sphere.e
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

[Postprocessors]
  [lambda]
    type = LambdaEffective
  []
  [lambda_rel]
    type = LambdaEffective
    output = rel_err
  []
  [lambda_std]
    type = LambdaEffective
    output = std_dev
  []
  [beta]
    type = BetaEffective
  []
  [beta_rel]
    type = BetaEffective
    output = rel_err
  []
  [beta_std]
    type = BetaEffective
    output = std_dev
  []
[]

[Outputs]
  csv = true
[]
