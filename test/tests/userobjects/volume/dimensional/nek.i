[Mesh]
  type = NekRSMesh
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'sfr_7pin'
  output = 'temperature pressure velocity'
[]

[AuxVariables]
  [avg_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_v]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_T]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [integral_v]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [avg_T]
    type = SpatialUserObjectAux
    variable = avg_T
    user_object = avg_T
  []
  [avg_p]
    type = SpatialUserObjectAux
    variable = avg_p
    user_object = avg_p
  []
  [avg_v]
    type = SpatialUserObjectAux
    variable = avg_v
    user_object = avg_v
  []
  [integral_T]
    type = SpatialUserObjectAux
    variable = integral_T
    user_object = integral_T
  []
  [integral_p]
    type = SpatialUserObjectAux
    variable = integral_p
    user_object = integral_p
  []
  [integral_v]
    type = SpatialUserObjectAux
    variable = integral_v
    user_object = integral_v
  []
[]

[UserObjects]
  [subchannel_binning]
    type = HexagonalSubchannelBin
    bundle_pitch = 0.02583914354890463
    pin_pitch = 0.0089656996
    pin_diameter = 7.646e-3
    n_rings = 2
  []
  [axial_binning]
    type = LayeredBin
    direction = z
    num_layers = 6
  []
  [avg_T]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning axial_binning'
    field = temperature
  []
  [avg_p]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning axial_binning'
    field = pressure
  []
  [avg_v]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning axial_binning'
    field = velocity
  []
  [integral_T]
    type = NekBinnedVolumeIntegral
    bins = 'subchannel_binning axial_binning'
    field = temperature
  []
  [integral_p]
    type = NekBinnedVolumeIntegral
    bins = 'subchannel_binning axial_binning'
    field = pressure
  []
  [integral_v]
    type = NekBinnedVolumeIntegral
    bins = 'subchannel_binning axial_binning'
    field = velocity
  []
[]

[MultiApps]
  [subchannel]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'subchannel.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [uo1_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = avg_T
    to_multi_app = subchannel
    variable = avg_T
  []
  [uo2_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = avg_p
    to_multi_app = subchannel
    variable = avg_p
  []
  [uo3_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = avg_v
    to_multi_app = subchannel
    variable = avg_v
  []
  [uo4_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = integral_T
    to_multi_app = subchannel
    variable = integral_T
  []
  [uo5_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = integral_p
    to_multi_app = subchannel
    variable = integral_p
  []
  [uo6_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = integral_v
    to_multi_app = subchannel
    variable = integral_v
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
  file_base = nek_dim
[]
