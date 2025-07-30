gap_thickness = ${fparse 0.1 * 7.646e-3}

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'

  [FieldTransfers]
    [P]
      type = NekFieldVariable
      direction = from_nek
      field = pressure
    []
  []
[]

[AuxVariables]
  [gap_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_p]
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
  [avg_p]
    type = SpatialUserObjectAux
    variable = avg_p
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
    type = NekBinnedPlaneAverage
    bins = 'subchannel_binning axial_binning'
    field = pressure
    gap_thickness = ${gap_thickness}
    map_space_by_qp = true
  []
  [gap_area]
    type = NekBinnedPlaneIntegral
    bins = 'subchannel_binning axial_binning'
    field = unity
    gap_thickness = ${gap_thickness}
    map_space_by_qp = true
  []
[]

[MultiApps]
  [subchannel]
    type = TransientMultiApp
    input_files = 'subchannel_axial.i'
    execute_on = timestep_end
  []
[]

[Transfers]
  [uo1_to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    source_user_object = gap_avg
    to_multi_app = subchannel
    variable = gap_avg
  []
  [uo2_to_sub]
    type = MultiAppGeneralFieldUserObjectTransfer
    source_user_object = gap_area
    to_multi_app = subchannel
    variable = gap_area
  []
  [temp_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = subchannel
    variable = P
    source_variable = P
  []
[]

[VectorPostprocessors]
  [avg_p]
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
