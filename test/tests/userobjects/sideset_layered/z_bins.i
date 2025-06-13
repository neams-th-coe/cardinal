[GlobalParams]
  check_boundary_restricted = false
  map_space_by_qp = true
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
[]

[AuxVariables]
  [uo1_bin1]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo1_bin3]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo2_bin1]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo2_bin3]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo_1_2_bin3]
    family = MONOMIAL
    order = CONSTANT
  []
  [T1_bin1]
    family = MONOMIAL
    order = CONSTANT
  []
  [T1_bin3]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [T1_bin1]
    type = SpatialUserObjectAux
    variable = T1_bin1
    user_object = T1_bin1
    boundary = '1'
  []
  [T1_bin3]
    type = SpatialUserObjectAux
    variable = T1_bin3
    user_object = T1_bin3
    boundary = '1'
  []
  [uo1_bin1]
    type = SpatialUserObjectAux
    variable = uo1_bin1
    user_object = side1_bin1
    boundary = '1'
  []
  [uo2_bin1]
    type = SpatialUserObjectAux
    variable = uo2_bin1
    user_object = side2_bin1
    boundary = '2'
  []
  [uo1_bin3]
    type = SpatialUserObjectAux
    variable = uo1_bin3
    user_object = side1_bin3
    boundary = '1'
  []
  [uo2_bin3]
    type = SpatialUserObjectAux
    variable = uo2_bin3
    user_object = side2_bin3
    boundary = '2'
  []
  [uo_1_2_bin3]
    type = SpatialUserObjectAux
    variable = uo_1_2_bin3
    user_object = side_1_2_bin3
    boundary = '1 2'
  []
[]

[UserObjects]
  [z1]
    type = LayeredBin
    direction = z
    num_layers = 1
  []
  [side1_bin1]
    type = NekBinnedSideIntegral
    bins = 'z1'
    field = unity
    boundary = '1'
  []
  [side2_bin1]
    type = NekBinnedSideIntegral
    bins = 'z1'
    field = unity
    boundary = '2'
  []
  [side_1_2_bin1] # should equal the sum of side1_bin1 and side1_bin2
    type = NekBinnedSideIntegral
    bins = 'z1'
    field = unity
    boundary = '1 2'
  []
  [T1_bin1]
    type = NekBinnedSideIntegral
    bins = 'z1'
    field = temperature
    boundary = '1'
  []

  [z3]
    type = LayeredBin
    direction = z
    num_layers = 3
  []
  [side1_bin3]
    type = NekBinnedSideIntegral
    bins = 'z3'
    field = unity
    boundary = '1'
  []
  [side2_bin3]
    type = NekBinnedSideIntegral
    bins = 'z3'
    field = unity
    boundary = '2'
  []
  [side_1_2_bin3] # should equal the sum of side1_bin3 and side2_bin3
    type = NekBinnedSideIntegral
    bins = 'z3'
    field = unity
    boundary = '1 2'
  []
  [T1_bin3]
    type = NekBinnedSideIntegral
    bins = 'z3'
    field = temperature
    boundary = '1'
  []
[]

[Postprocessors]
  [area1] # should match the value of the side1_bin1 user object
    type = NekSideIntegral
    field = unity
    boundary = '1'
  []
  [area_divided1] # should match the value of the side1_bin3 user object
    type = LinearCombinationPostprocessor
    pp_names = 'area1'
    pp_coefs = '${fparse 1.0 / 3.0}'
  []
  [area2] # should match the value of the side2_bin1 user object
    type = NekSideIntegral
    field = unity
    boundary = '2'
  []
  [area_divided2] # should match the value of the side2_bin3 user object
    type = LinearCombinationPostprocessor
    pp_names = 'area2'
    pp_coefs = '${fparse 1.0 / 3.0}'
  []
  [T1] # should match the value of the T1_bin1 user object
    type = NekSideIntegral
    field = temperature
    boundary = '1'
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  execute_on = 'final'
  exodus = true
[]
