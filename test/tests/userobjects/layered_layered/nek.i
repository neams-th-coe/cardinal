[Problem]
  type = NekRSProblem
  casename = 'brick'

  [Dimensionalize]
    L = 2.0
    U = 1.0
    rho = 834.5
    Cp = 1228.0
    T = 573.0
    dT = 10.0
  []

  [FieldTransfers]
    [P]
      type = NekFieldVariable
      direction = from_nek
      field = pressure
    []
  []
[]

[Mesh]
  type = NekRSMesh
  volume = true
  scaling = 2.0
[]

[AuxVariables]
  [bin_volumes]
    family = MONOMIAL
    order = CONSTANT
  []
  [avg_p]
    family = MONOMIAL
    order = CONSTANT
  []
  [total_volume]
    family = MONOMIAL
    order = CONSTANT
  []
  [total_average_p]
    family = MONOMIAL
    order = CONSTANT
  []

  # just for visualization of the binning
  [x_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [y_bins]
    family = MONOMIAL
    order = CONSTANT
  []
  [z_bins]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  # just for visualization of the binning
  [x_bins]
    type = SpatialUserObjectAux
    variable = x_bins
    user_object = x_bins
    execute_on = INITIAL
  []
  [y_bins]
    type = SpatialUserObjectAux
    variable = y_bins
    user_object = y_bins
    execute_on = INITIAL
  []
  [z_bins]
    type = SpatialUserObjectAux
    variable = z_bins
    user_object = z_bins
    execute_on = INITIAL
  []

  [bin_volumes]
    type = SpatialUserObjectAux
    variable = bin_volumes
    user_object = vol_integral
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [avg_p]
    type = SpatialUserObjectAux
    variable = avg_p
    user_object = avg_p
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_volume]
    type = SpatialUserObjectAux
    variable = total_volume
    user_object = reference_vol_integral
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [total_average_p]
    type = SpatialUserObjectAux
    variable = total_average_p
    user_object = reference_pressure_avg
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[UserObjects]
  [x_bins]
    type = LayeredBin
    direction = x
    num_layers = 3
  []
  [y_bins]
    type = LayeredBin
    direction = y
    num_layers = 3
  []
  [z_bins]
    type = LayeredBin
    direction = z
    num_layers = 12
  []
  [vol_integral]
    type = NekBinnedVolumeIntegral
    bins = 'x_bins y_bins z_bins'
    field = unity
  []
  [avg_p]
    type = NekBinnedVolumeAverage
    bins = 'x_bins y_bins z_bins'
    field = pressure
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
  [reference_pressure_avg]
    type = NekBinnedVolumeAverage
    bins = 'one_bin'
    field = pressure
  []
[]

[Postprocessors]
  # we compare the integral (with a single bin) with an already-verified postprocessor
  # to make sure the actual internals of the binned volume integral are done correctly
  [volume_ref] # should match the value in 'total_volume' (computed with 1 bin)
    type = NekVolumeIntegral
    field = unity
  []
  [avg_p_ref] # should match the value in 'total_average_p' (computed with 1 bin)
    type = NekVolumeAverage
    field = pressure
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
