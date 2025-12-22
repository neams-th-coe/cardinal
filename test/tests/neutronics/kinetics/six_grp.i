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

  [beta_1]
    type = BetaEffective
    beta_type = '1'
  []
  [beta_1_rel]
    type = BetaEffective
    output = rel_err
    beta_type = '1'
  []
  [beta_1_std]
    type = BetaEffective
    output = std_dev
    beta_type = '1'
  []

  [beta_2]
    type = BetaEffective
    beta_type = '2'
  []
  [beta_2_rel]
    type = BetaEffective
    output = rel_err
    beta_type = '2'
  []
  [beta_2_std]
    type = BetaEffective
    output = std_dev
    beta_type = '2'
  []

  [beta_3]
    type = BetaEffective
    beta_type = '3'
  []
  [beta_3_rel]
    type = BetaEffective
    output = rel_err
    beta_type = '3'
  []
  [beta_3_std]
    type = BetaEffective
    output = std_dev
    beta_type = '3'
  []

  [beta_4]
    type = BetaEffective
    beta_type = '4'
  []
  [beta_4_rel]
    type = BetaEffective
    output = rel_err
    beta_type = '4'
  []
  [beta_4_std]
    type = BetaEffective
    output = std_dev
    beta_type = '4'
  []

  [beta_5]
    type = BetaEffective
    beta_type = '5'
  []
  [beta_5_rel]
    type = BetaEffective
    output = rel_err
    beta_type = '5'
  []
  [beta_5_std]
    type = BetaEffective
    output = std_dev
    beta_type = '5'
  []

  [beta_6]
    type = BetaEffective
    beta_type = '6'
  []
  [beta_6_rel]
    type = BetaEffective
    output = rel_err
    beta_type = '6'
  []
  [beta_6_std]
    type = BetaEffective
    output = std_dev
    beta_type = '6'
  []
[]

[Outputs]
  csv = true
[]
