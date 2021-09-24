[Mesh]
  type = HexagonalSubchannelMesh
  bundle_pitch = 0.02583914354890463
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 2
  n_axial = 7
  theta_res = 8
  height = 0.008
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [average_T]
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
