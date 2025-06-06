[Problem]
  type = NekRSProblem
  casename = 'cylinder'

  [Dimensionalize]
    L = 5.0
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
  scaling = 5.0
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
  [r_bins]
    type = RadialBin
    vertical_axis = z
    nr = 5
    rmin = 0.0
    rmax = 1.0
  []
  [vol_integral]
    type = NekBinnedVolumeIntegral
    bins = 'r_bins'
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
    points = '0.1 0.0 0.0
              0.3 0.0 0.0
              0.5 0.0 0.0
              0.7 0.0 0.0
              0.9 0.0 0.0'
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
