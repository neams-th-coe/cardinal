[Mesh]
  type = HexagonalSubchannelMesh
  bundle_pitch = 1.0
  pin_pitch = 0.8
  pin_diameter = 0.6
  n_rings = 1
  n_axial = 2
  height = 0.5
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
