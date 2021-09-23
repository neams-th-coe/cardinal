bundle_pitch = 0.04

[Mesh]
  type = HexagonalSubchannelMesh
  bundle_pitch = ${bundle_pitch}
  pin_pitch = 0.0089656996
  pin_diameter = 7.646e-3
  n_rings = 3
  n_axial = 1
  theta_res = 5
  height = 0.005
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [bin]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [bin]
    type = SpatialUserObjectAux
    variable = bin
    user_object = subchannel_bins
  []
[]

[UserObjects]
  [subchannel_bins]
    type = HexagonalSubchannelBin
    bundle_pitch = ${bundle_pitch}
    pin_pitch = 0.0089656996
    pin_diameter = 7.646e-3
    n_rings = 3
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
