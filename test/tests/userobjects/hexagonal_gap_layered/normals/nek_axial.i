gap_thickness = ${fparse 0.05 * 7.646e-3}

[Mesh]
  type = NekRSMesh
  volume = true
  parallel_type = replicated
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'sfr_7pin'
  output = 'velocity'
[]

[AuxVariables]
  # These are just for visualizing the average velocity component with Glyphs in paraview;
  # the result of the 'vol_avg' user object will be represented as a vector "uo_" with 3 components
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
    user_object = avg_velocity_component
    component = 0
  []
  [uo_y]
    type = NekSpatialBinComponentAux
    variable = uo_y
    user_object = avg_velocity_component
    component = 1
  []
  [uo_z]
    type = NekSpatialBinComponentAux
    variable = uo_z
    user_object = avg_velocity_component
    component = 2
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
  [avg_velocity_component]
    type = NekBinnedPlaneAverage
    bins = 'subchannel_binning axial_binning'
    field = velocity_component
    velocity_component = normal
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
  [uox_to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = subchannel
    source_variable = uo_x
    variable = uo_x
  []
  [uoy_to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = subchannel
    source_variable = uo_y
    variable = uo_y
  []
  [uoz_to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = subchannel
    source_variable = uo_z
    variable = uo_z
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
