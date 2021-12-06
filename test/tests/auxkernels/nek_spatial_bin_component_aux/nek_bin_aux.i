[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Problem]
  type = FEProblem
  solve = false
[]

[UserObjects]
  [dummy]
    type = NearestNodeNumberUO
    point = '0.1 0.1 0.1'
  []
[]

[AuxVariables]
  [dummy]
  []
[]

[AuxKernels]
  [error]
    type = NekSpatialBinComponentAux
    variable = dummy
    user_object = dummy
    component = 0
  []
[]

[Executioner]
  type = Steady
[]
