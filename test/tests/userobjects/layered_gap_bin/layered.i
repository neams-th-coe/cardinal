[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
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
    type = LayeredGapBin
    num_layers = 5
    direction = x
  []
  [y_bins]
    type = LayeredGapBin
    num_layers = 5
    direction = y
  []
  [z_bins]
    type = LayeredGapBin
    num_layers = 2
    direction = z
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
