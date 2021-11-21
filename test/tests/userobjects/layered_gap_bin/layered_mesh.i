[Mesh]
  [plane]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
  []
  [repeat]
    type = CombinerGenerator
    inputs = plane
    positions = '0.0 0.0 0.0
                 0.0 0.0 1.0
                 0.0 0.0 2.0
                 0.0 0.0 3.0
                 0.0 0.0 4.0'
  []
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [z_bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [z_bins]
    type = SpatialUserObjectAux
    variable = z_bins
    user_object = z_bins
  []
[]

[UserObjects]
  [z_bins]
    type = LayeredGapBin
    num_layers = 5
    direction = z
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
