[Problem]
  type = NekRSProblem
  casename = 'brick'

  [Dimensionalize]
    L = 2.0
    U = 1.0
    rho = 834.5
    Cp = 1228.0
    T = 573.0
    dT = 10.0
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  scaling = 2.0
[]

[AuxVariables]
  [uo]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [uo]
    type = SpatialUserObjectAux
    variable = uo
    user_object = vol_integral
  []
[]

[UserObjects]
  [x_bins]
    type = LayeredBin
    direction = x
    num_layers = 3
  []
  [y_bins]
    type = LayeredBin
    direction = y
    num_layers = 3
  []
  [z_bins]
    type = LayeredBin
    direction = z
    num_layers = 4
  []
  [vol_integral]
    type = NekBinnedVolumeIntegral
    bins = 'x_bins y_bins z_bins'
    field = pressure
  []
[]

[VectorPostprocessors]
  # from_uo gives exactly the same results as manually specifying the points in 'manually_provided'
  [from_uo]
    type = SpatialUserObjectVectorPostprocessor
    userobject = vol_integral
  []
  [manually_provided]
    type = SpatialUserObjectVectorPostprocessor
    userobject = vol_integral
    points = '-0.66666667 -0.66666667 -3.0
              -0.66666667 -0.66666667 -1.0
              -0.66666667 -0.66666667  1.0
              -0.66666667 -0.66666667  3.0

              -0.66666667 0.0         -3.0
              -0.66666667 0.0         -1.0
              -0.66666667 0.0          1.0
              -0.66666667 0.0          3.0

              -0.66666667 0.66666667 -3.0
              -0.66666667 0.66666667 -1.0
              -0.66666667 0.66666667  1.0
              -0.66666667 0.66666667  3.0

               0.0       -0.66666667 -3.0
               0.0       -0.66666667 -1.0
               0.0       -0.66666667  1.0
               0.0       -0.66666667  3.0

               0.0        0.0        -3.0
               0.0        0.0        -1.0
               0.0        0.0         1.0
               0.0        0.0         3.0

               0.0        0.66666667 -3.0
               0.0        0.66666667 -1.0
               0.0        0.66666667  1.0
               0.0        0.66666667  3.0

               0.66666667 -0.66666667 -3.0
               0.66666667 -0.66666667 -1.0
               0.66666667 -0.66666667  1.0
               0.66666667 -0.66666667  3.0

               0.66666667 0.0         -3.0
               0.66666667 0.0         -1.0
               0.66666667 0.0          1.0
               0.66666667 0.0          3.0

               0.66666667 0.66666667 -3.0
               0.66666667 0.66666667 -1.0
               0.66666667 0.66666667  1.0
               0.66666667 0.66666667  3.0'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = 'final'
[]
