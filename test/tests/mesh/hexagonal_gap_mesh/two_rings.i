[Mesh]
  type = HexagonalSubchannelGapMesh
  bundle_pitch = 2.5
  pin_pitch = 0.8
  pin_diameter = 0.6
  n_rings = 2
  n_axial = 3
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
