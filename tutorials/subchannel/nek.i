[Mesh]
  type = NekRSMesh
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'sfr_7pin'
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
    num_layers = 7
  []
  [average_T]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning axial_binning'
    field = temperature
    map_space_by_qp = true
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
  [uo_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = average_T
    direction = to_multiapp
    multi_app = subchannel
    variable = average_T
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
