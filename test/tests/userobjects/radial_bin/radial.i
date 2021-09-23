[Mesh]
  [disc]
    type = AnnularMeshGenerator
    nr = 100
    nt = 24
    rmin = 0.0
    rmax = 1.5
  []
[]

[Problem]
  solve = false
  type = FEProblem
[]

[AuxVariables]
  [uniform_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [growing_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [shrinking_bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [uniform_bins]
    type = SpatialUserObjectAux
    variable = uniform_bins
    user_object = uniform_bins
  []
  [growing_bins]
    type = SpatialUserObjectAux
    variable = growing_bins
    user_object = growing_bins
  []
  [shrinking_bins]
    type = SpatialUserObjectAux
    variable = shrinking_bins
    user_object = shrinking_bins
  []
[]

[UserObjects]
  [uniform_bins]
    type = RadialBin
    rmin = 0.0
    rmax = 1.5
    nr = 10
    vertical_axis = z
  []
  [growing_bins]
    type = RadialBin
    rmin = 0.0
    rmax = 1.5
    nr = 10
    growth_r = 1.2
    vertical_axis = z
  []
  [shrinking_bins]
    type = RadialBin
    rmin = 0.0
    rmax = 1.5
    nr = 10
    growth_r = 0.8
    vertical_axis = z
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
