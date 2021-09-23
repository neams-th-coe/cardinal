[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 8
  ny = 8
  nz = 8
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [x_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [y_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [z_bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [x_bins]
    type = SpatialUserObjectAux
    variable = x_bins
    user_object = x_bins
  []
  [y_bins]
    type = SpatialUserObjectAux
    variable = y_bins
    user_object = y_bins
  []
  [z_bins]
    type = SpatialUserObjectAux
    variable = z_bins
    user_object = z_bins
  []
[]

[UserObjects]
  [x_bins]
    type = LayeredBin
    num_layers = 4
    direction = x
  []
  [y_bins]
    type = LayeredBin
    num_layers = 5
    direction = y
  []
  [z_bins]
    type = LayeredBin
    num_layers = 6
    direction = z
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
