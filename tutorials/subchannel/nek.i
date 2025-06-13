[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'

  [FieldTransfers]
    [temperature]
      type = NekFieldVariable
      direction = from_nek
    []
    [velocity_x]
      type = NekFieldVariable
      direction = from_nek
    []
    [velocity_y]
      type = NekFieldVariable
      direction = from_nek
    []
    [velocity_z]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_z
    []
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
  [subchannel_gap_binning]
    type = HexagonalSubchannelGapBin
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
  [average_T_gaps]
    type = NekBinnedPlaneAverage
    bins = 'subchannel_gap_binning axial_binning'
    field = temperature
    map_space_by_qp = true
    gap_thickness = ${fparse 0.05 * 7.646e-3}
  []
  [avg_gap_velocity]
    type = NekBinnedPlaneAverage
    bins = 'subchannel_gap_binning axial_binning'
    field = velocity_component
    velocity_component = normal
    map_space_by_qp = true
    gap_thickness = ${fparse 0.05 * 7.646e-3}
  []
[]

[AuxVariables]
  # These are just for visualizing the average velocity component with Glyphs in paraview;
  # the result of the 'avg_gap_velocity' user object will be represented as a vector "uo_" with 3 components
  [uo_x]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo_y]
    family = MONOMIAL
    order = CONSTANT
  []
  [uo_z]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [uo_x]
    type = NekSpatialBinComponentAux
    variable = uo_x
    user_object = avg_gap_velocity
    component = 0
  []
  [uo_y]
    type = NekSpatialBinComponentAux
    variable = uo_y
    user_object = avg_gap_velocity
    component = 1
  []
  [uo_z]
    type = NekSpatialBinComponentAux
    variable = uo_z
    user_object = avg_gap_velocity
    component = 2
  []
[]

[MultiApps]
  [subchannel]
    type = TransientMultiApp
    input_files = 'subchannel.i'
    execute_on = timestep_end
    sub_cycling = true
  []
  [subchannel_gap]
    type = TransientMultiApp
    input_files = 'subchannel_gap.i'
    execute_on = timestep_end
    sub_cycling = true
  []
[]

[Transfers]
  [uo_to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    source_user_object = average_T
    to_multi_app = subchannel
    variable = average_T
  []
  [uo_to_sub2]
    type = MultiAppGeneralFieldUserObjectTransfer
    source_user_object = average_T_gaps
    to_multi_app = subchannel_gap
    variable = average_T
  []
  [uo1_to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    source_user_object = avg_gap_velocity
    to_multi_app = subchannel_gap
    variable = avg_gap_velocity
  []
  [uox_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = subchannel_gap
    source_variable = uo_x
    variable = uo_x
  []
  [uoy_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = subchannel_gap
    source_variable = uo_y
    variable = uo_y
  []
[]

[VectorPostprocessors]
  [avg_T]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_T
  []
  [avg_T_gaps]
    type = SpatialUserObjectVectorPostprocessor
    userobject = average_T_gaps
  []
  [avg_v_gaps]
    type = SpatialUserObjectVectorPostprocessor
    userobject = avg_gap_velocity
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
[]
