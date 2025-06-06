[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
[]

[AuxVariables]
  [subchannel_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [axial_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [total_volume]
    family = MONOMIAL
    order = CONSTANT
  []
  [total_average_T]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [bins1]
    type = SpatialUserObjectAux
    variable = subchannel_bins
    user_object = subchannel_binning
  []
  [bins2]
    type = SpatialUserObjectAux
    variable = axial_bins
    user_object = axial_binning
  []
  [total_volume]
    type = SpatialUserObjectAux
    variable = total_volume
    user_object = reference_vol_integral
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_average_T]
    type = SpatialUserObjectAux
    variable = total_average_T
    user_object = reference_T_avg
    execute_on = 'INITIAL TIMESTEP_END'
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
  [vol_avg]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning axial_binning'
    field = temperature
  []
  [vol_integral]
    type = NekBinnedVolumeIntegral
    bins = 'subchannel_binning axial_binning'
    field = unity
  []

  [one_bin]
    type = LayeredBin
    direction = z
    num_layers = 1
  []
  [reference_vol_integral]
    type = NekBinnedVolumeIntegral
    bins = 'one_bin'
    field = unity
  []
  [reference_T_avg]
    type = NekBinnedVolumeAverage
    bins = 'one_bin'
    field = temperature
  []
[]

[MultiApps]
  [subchannel]
    type = TransientMultiApp
    input_files = 'subchannel.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [uo1_to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    source_user_object = vol_avg
    to_multi_app = subchannel
    variable = vol_avg
  []
  [uo2_to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    source_user_object = vol_integral
    to_multi_app = subchannel
    variable = vol_integral
  []
[]

[Postprocessors]
  # we compare the integral (with a single bin) with an already-verified postprocessor
  # to make sure the actual internals of the binned volume integral are done correctly
  [volume_ref] # should match the value in 'total_volume' (computed with 1 bin)
    type = NekVolumeIntegral
    field = unity
  []
  [avg_T_ref] # should match the value in 'total_average_T' (computed with 1 bin)
    type = NekVolumeAverage
    field = temperature
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
[]
