[GlobalParams]
  map_space_by_qp = true
  check_boundary_restricted = false
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'sfr_7pin'
  output = 'temperature'
[]

[AuxVariables]
  [avg_T_duct_wall]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [avg_T_duct_wall]
    type = SpatialUserObjectAux
    variable = avg_T_duct_wall
    user_object = avg_T_duct_wall
    boundary = '2'
  []
[]

[UserObjects]
  [x]
    type = LayeredBin
    direction = x
    num_layers = 2
  []
  [y]
    type = LayeredBin
    direction = y
    num_layers = 2
  []
  [z]
    type = LayeredBin
    direction = z
    num_layers = 3
  []
  [avg_T_duct_wall]
    type = NekBinnedSideAverage
    bins = 'x y z'
    field = temperature
    boundary = '2'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[VectorPostprocessors]
  [temp_duct]
    type = SpatialUserObjectVectorPostprocessor
    userobject = avg_T_duct_wall
  []
[]

[Outputs]
  csv = true
  exodus = true
[]
