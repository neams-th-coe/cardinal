[Mesh]
  type = NekRSMesh
  volume = true
  order = SECOND
[]

[Problem]
  type = NekRSProblem
  casename = 'turbPipe'

  # normally, Cardinal will allocate the usrwrk space for you; but in this
  # standalone case, we decided to allocate nrs->usrwrk ourselves already in the
  # udf file
  n_usrwrk_slots = 0

  [FieldTransfers]
    [pressure]
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
    []
  []
[]

[Postprocessors]
  [outlet_p]
    type = NekSideAverage
    boundary = '2'
    field = pressure
  []
  [inlet_p]
    type = NekSideAverage
    boundary = '1'
    field = pressure
  []
  [mdot]
    type = NekMassFluxWeightedSideIntegral
    boundary = '1'
    field = unity
  []

  # subtracts the two pressure postprocessors
  [dP]
    type = DifferencePostprocessor
    value1 = outlet_p
    value2 = inlet_p
  []
[]

[UserObjects]
  [axial_bins]
    type = LayeredBin
    direction = z
    num_layers = 20
  []
  [radial_bins]
    type = RadialBin
    vertical_axis = z
    rmax = 0.5
    nr = 12
    growth_r = 0.9
  []
  [volume_averages]
    type = NekBinnedVolumeAverage
    bins = 'radial_bins axial_bins'
    field = velocity_z
    map_space_by_qp = true
  []
[]

[AuxVariables]
  [volume_averages]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [volume_averages]
    type = SpatialUserObjectAux
    variable = volume_averages
    user_object = volume_averages
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = timestep_end
  []
[]

[Transfers]
  [uo_to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    to_multi_app = sub
    source_user_object = volume_averages
    variable = avg_velocity
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

  # this hides these values from the screen for neater output
  hide = 'outlet_p inlet_p'
[]
