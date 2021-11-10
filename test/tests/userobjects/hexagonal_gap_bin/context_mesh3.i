bundle_pitch = 0.04

[Mesh]
  type = HexagonalSubchannelMesh
  bundle_pitch = ${bundle_pitch}
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 3
  n_axial = 1
  height = 0.005
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [u]
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
