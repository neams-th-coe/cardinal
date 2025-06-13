[GlobalParams]
  check_boundary_restricted = false
  map_space_by_qp = false
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
  [T1_bin3]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [T1_bin3]
    type = SpatialUserObjectAux
    variable = T1_bin3
    user_object = T1_bin3
    boundary = '1'
  []
[]

[UserObjects]
  [z3]
    type = LayeredBin
    direction = z
    num_layers = 3
  []
  [T1_bin3]
    type = NekBinnedSideIntegral
    bins = 'z3'
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

[VectorPostprocessors]
  [T]
    type = SpatialUserObjectVectorPostprocessor
    userobject = T1_bin3
  []
[]

[Outputs]
  exodus = true
[]
