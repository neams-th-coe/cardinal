[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
[]

[AuxVariables]
  [uo]
  []
[]

[AuxKernels]
  [uo]
    type = SpatialUserObjectAux
    variable = uo
    user_object = side_avg
    boundary = '1'
  []
[]

[UserObjects]
  [subchannel_binning]
    type = HexagonalSubchannelBin
    bundle_pitch = 0.02583914354890463
    pin_pitch = 0.0089656996
    pin_diameter = 7.646e-3
    n_rings = 2
    pin_centered_bins = true
  []
  [side_avg]
    type = NekBinnedSideAverage
    bins = 'subchannel_binning'
    field = temperature
    boundary = '1'

    # for the peripheral region, we wont actually hit anything in this zone because we evaluate
    # the user object on the pin surfaces
    check_zero_contributions = false
  []
[]

[VectorPostprocessors]
  # from_uo gives exactly the same results as manually specifying the points in 'manually_provided',
  # but without the headache of figuring out what all the centroids of the pins are
  [from_uo]
    type = SpatialUserObjectVectorPostprocessor
    userobject = side_avg
  []
  [manually_provided]
    type = SpatialUserObjectVectorPostprocessor
    userobject = side_avg
    points = ' 0.0        0.0       0.0
               0.0044828  0.0077645 0.0
              -0.0044828  0.0077645 0.0
              -0.0089656  0.0       0.0
              -0.0044828 -0.0077645 0.0
               0.0044828 -0.0077645 0.0
               0.0089656  0.0       0.0
               1.0        0.0       0.0' # last point is just any point outside the pin area
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
  execute_on = 'final'
[]
