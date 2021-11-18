gap_thickness = ${fparse 0.05 * 7.646e-3}

[Mesh]
  type = NekRSMesh
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'sfr_7pin'
  output = 'temperature'
[]

[AuxVariables]
  [gap_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_temp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [gap_bins]
    type = SpatialUserObjectAux
    variable = gap_bins
    user_object = subchannel_binning
  []
  [avg_temp]
    type = SpatialUserObjectAux
    variable = avg_temp
    user_object = gap_avg
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
    type = LayeredGapBin
    direction = z
    num_layers = 6
  []
  [gap_avg]
    type = NekBinnedSideAverage
    bins = 'subchannel_binning axial_binning'
    field = temperature
    gap_thickness = ${gap_thickness}
    map_space_by_qp = true
  []
  [gap_area]
    type = NekBinnedSideIntegral
    bins = 'subchannel_binning axial_binning'
    field = unity
    gap_thickness = ${gap_thickness}
    map_space_by_qp = true
  []
[]

[MultiApps]
  [subchannel]
    type = TransientMultiApp
    app_type = CardinalApp
    input_files = 'subchannel_axial.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [uo1_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = gap_avg
    direction = to_multiapp
    multi_app = subchannel
    variable = gap_avg
  []
  [uo2_to_sub]
    type = MultiAppUserObjectTransfer
    user_object = gap_area
    direction = to_multiapp
    multi_app = subchannel
    variable = gap_area
  []
  [temp_to_sub]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    multi_app = subchannel
    variable = temp
    source_variable = temp
  []
[]

[VectorPostprocessors]
  [avg_temp]
    type = SpatialUserObjectVectorPostprocessor
    userobject = gap_avg
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
  csv = true
[]
