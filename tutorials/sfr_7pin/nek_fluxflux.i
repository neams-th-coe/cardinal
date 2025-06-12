interval = 100

[Mesh]
  type = NekRSMesh
  boundary = '1 2'
[]

[Problem]
  type = NekRSProblem
  casename = 'sfr_7pin'
  synchronization_interval = parent_app

  [FieldTransfers]
    [heat_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
    []
    [temperature]
      type = NekFieldVariable
      direction = from_nek
    []
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

  # this will only display the NekRS output every N time steps; postprocessors
  # are still computed on every step, just not output to the console
  time_step_interval = ${interval}
[]

[UserObjects]
  [axial_bin]
    type = LayeredBin
    direction = z
    num_layers = 20
  []
  [volume_bin]
    type = HexagonalSubchannelBin
    bundle_pitch = ${fparse 0.02625*1.1}
    pin_pitch = 0.00904000030292588
    pin_diameter = 8e-3
    n_rings = 2
  []
  [wall_flux]
    type = NekBinnedSideAverage
    bins = 'axial_bin volume_bin'
    boundary = '1'
    field = usrwrk00
    map_space_by_qp = true
    interval = ${interval}
    check_zero_contributions = false
  []
  [wall_temp]
    type = NekBinnedSideAverage
    bins = 'axial_bin volume_bin'
    boundary = '1'
    field = temperature
    map_space_by_qp = true
    interval = ${interval}
    check_zero_contributions = false
  []
  [bulk_temp]
    type = NekBinnedVolumeAverage
    bins = 'axial_bin volume_bin'
    field = temperature
    map_space_by_qp = true
    interval = ${interval}
    check_zero_contributions = false
  []
[]

[AuxVariables]
  [h]
  []
[]

[AuxKernels]
  [h]
    type = HeatTransferCoefficientAux
    variable = h
    wall_T = wall_temp
    bulk_T = bulk_temp
    heat_flux = wall_flux
  []
[]

[Postprocessors]
  [pin_flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
  [duct_flux_in_nek]
    type = NekHeatFluxIntegral
    boundary = '2'
  []
  [max_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_nek_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
[]
