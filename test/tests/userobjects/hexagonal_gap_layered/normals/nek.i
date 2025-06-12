gap_thickness = ${fparse 0.05 * 7.646e-3}

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'

  [FieldTransfers]
    [vel_x]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_x
    []
    [vel_y]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_y
    []
    [vel_z]
      type = NekFieldVariable
      direction = from_nek
      field = velocity_z
    []
  []
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
    type = HexagonalSubchannelGapBin
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
  [avg_velocity_component]
    type = NekBinnedPlaneAverage
    bins = 'subchannel_binning'
    field = velocity_component
    velocity_component = normal
    gap_thickness = ${gap_thickness}
    map_space_by_qp = true
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
    source_user_object = avg_velocity_component
    to_multi_app = subchannel
    variable = avg_velocity_component
  []
  [uox_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = subchannel
    source_variable = uo_x
    variable = uo_x
    search_value_conflicts = false
  []
  [uoy_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = subchannel
    source_variable = uo_y
    variable = uo_y
    search_value_conflicts = false
  []
  [uoz_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = subchannel
    source_variable = uo_z
    variable = uo_z
    search_value_conflicts = false
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
