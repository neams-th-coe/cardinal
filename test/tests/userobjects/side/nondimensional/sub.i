[Mesh]
  [f]
    type = FileMeshGenerator
    file = nek_in.e
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[AuxVariables]
  [avg_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_v]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_v]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Executioner]
  type = Transient
[]

[Outputs]
  exodus = true
[]
