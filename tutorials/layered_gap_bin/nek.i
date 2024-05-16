[Problem]
  type = NekRSStandaloneProblem
  casename = 'periodic_duct'
  output = 'pressure'

  nondimensional = true
  L_ref = 8.758793E-03
  U_ref = 1.0
  rho_0 = 1.0
  Cp_0 = 1228.0
  T_ref = 573.0
  dT_ref = 10.0
[]

[Mesh]
  type = NekRSMesh
  volume = true
  exact = true
  order = FIRST
  scaling = 8.758793E-03
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
    user_object = pressure_planar_integral
  []
[]

[UserObjects]
  [z_bins]
    type = LayeredGapBin
    direction = z
    num_layers = 3
  []
  [pressure_planar_integral]
    type = NekBinnedPlaneIntegral
    bins = 'z_bins'
    field = pressure
    gap_thickness = 20.0
    map_space_by_qp = true
  []
  [unity_planar_integral]
    type = NekBinnedPlaneIntegral
    bins = 'z_bins'
    field = unity
    gap_thickness = 20.0
    map_space_by_qp = true
  []
[]

[VectorPostprocessors]
  # from_uo gives exactly the same results as manually specifying the points in 'manually_provided'
  [from_uo]
    type = SpatialUserObjectVectorPostprocessor
    userobject = pressure_planar_integral
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
