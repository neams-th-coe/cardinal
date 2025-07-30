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
  [velocity_component]
  []

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
  [velocity_component] # actual velocity component computed directly from interpolated nekRS velocity fields
    type = ParsedAux
    variable = velocity_component
    expression = '(0.1*vel_x-0.2*vel_y+0.3*vel_z)/sqrt(0.1*0.1+0.2*0.2+0.3*0.3)'
    coupled_variables = 'vel_x vel_y vel_z'
    execute_on = 'timestep_end nonlinear linear'
  []
  [uo_x]
    type = NekSpatialBinComponentAux
    variable = uo_x
    user_object = vol_avg
    component = 0
  []
  [uo_y]
    type = NekSpatialBinComponentAux
    variable = uo_y
    user_object = vol_avg
    component = 1
  []
  [uo_z]
    type = NekSpatialBinComponentAux
    variable = uo_z
    user_object = vol_avg
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
    type = LayeredBin
    direction = z
    num_layers = 6
  []
  [vol_avg]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning axial_binning'
    field = velocity_component
    velocity_component = user
    velocity_direction = '0.1 -0.2 0.3'
  []
[]

[MultiApps]
  [subchannel]
    type = TransientMultiApp
    input_files = 'subchannel_b.i'
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
  [uox_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = uo_x
    to_multi_app = subchannel
    variable = uo_x
  []
  [uoy_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = uo_y
    to_multi_app = subchannel
    variable = uo_y
  []
  [uoz_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = uo_z
    to_multi_app = subchannel
    variable = uo_z
  []
  [analytic_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    source_variable = velocity_component
    to_multi_app = subchannel
    variable = velocity_component
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

  # unhide and turn the number of time steps in the .par file to greater than 1 in order to see this
  # match the user object
  hide = 'velocity_component'
[]
