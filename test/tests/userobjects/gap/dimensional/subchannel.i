[Mesh]
  type = HexagonalSubchannelGapMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 6
  height = 0.008
[]

[Problem]
  solve = false
  type = FEProblem
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
