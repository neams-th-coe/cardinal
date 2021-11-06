[Mesh]
  type = HexagonalSubchannelGapMesh
  bundle_pitch = 4.0
  pin_pitch = 0.8
  pin_diameter = 0.6
  n_rings = 3
  n_axial = 10
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
