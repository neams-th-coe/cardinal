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
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [uo]
    type = SpatialUserObjectAux
    variable = uo
    user_object = vol_avg
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
  [vol_avg]
    type = NekBinnedVolumeAverage
    bins = 'subchannel_binning'
    field = temperature
  []
[]

[VectorPostprocessors]
  # from_uo gives exactly the same results as manually specifying the points in 'manually_provided',
  # but without the headache of figuring out what all the centroids of the subchannels are
  [from_uo]
    type = SpatialUserObjectVectorPostprocessor
    userobject = vol_avg
  []
  [manually_provided]
    type = SpatialUserObjectVectorPostprocessor
    userobject = vol_avg
    points = '         0  0.00517635        0
             -0.00448285  0.00258817        0
             -0.00448285 -0.00258817        0
                       0 -0.00517635        0
              0.00448285 -0.00258817        0
              0.00448285  0.00258817        0
                       0  0.010342          0
             -0.00895648  0.00517102        0
             -0.00895648 -0.00517102        0
                       0 -0.010342          0
              0.00895648 -0.00517102        0
              0.00895648  0.00517102        0
              0.00634302  0.0109864         0
             -0.00634302  0.0109864         0
             -0.01268600  0                 0
             -0.00634302 -0.0109864         0
              0.00634302 -0.0109864         0
              0.01268600  0                 0'
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
