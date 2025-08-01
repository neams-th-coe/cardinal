[Problem]
  type = NekRSProblem
  casename = 'sfr_pin'
  n_usrwrk_slots = 1

  # This input is run in nondimensional form to verify that all the postprocessors
  # and data transfers in/out of nekRS are properly dimensionalized.
  [Dimensionalize]
    U = 0.0950466
    T = 628.15
    dT = 50.0
    L = 0.908e-2
    rho = 834.5
    Cp = 1228.0
  []

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = flux_integral
    []
    [temp]
      type = NekFieldVariable
      field = temperature
      direction = from_nek
    []
  []
[]

[Mesh]
  type = NekRSMesh
  boundary = '1'

  # nekRS runs in non-dimensional form, which means that we shrunk the mesh
  # from physical units of meters to our characteristic scale of 0.908e-2 m
  # (the pin pitch, arbitrarily chosen). That means that we must multiply
  # the nekRS mesh by 0.908e-2 to get back in units of meters that BISON is
  # running in.
  scaling = 0.908e-2
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [synchronization_in]
    type = Receiver
  []

  # side integral
  [area_1]
    type = NekSideIntegral
    field = unity
    boundary = '1'
  []
  [pressure_1]
    type = NekSideIntegral
    field = pressure
    boundary = '1'
  []
  [temperature_1]
    type = NekSideIntegral
    field = temperature
    boundary = '1'
  []

  # side average
  [avg_area_1]
    type = NekSideAverage
    field = unity
    boundary = '1'
  []
  [avg_pressure_1]
    type = NekSideAverage
    field = pressure
    boundary = '1'
  []
  [avg_temperature_1]
    type = NekSideAverage
    field = temperature
    boundary = '1'
  []

  # volume integral
  [volume]
    type = NekVolumeIntegral
    field = unity
  []
  [pressure_vol]
    type = NekVolumeIntegral
    field = pressure
  []
  [temperature_vol]
    type = NekVolumeIntegral
    field = temperature
  []

  # volume average
  [avg_volume]
    type = NekVolumeAverage
    field = unity
  []
  [avg_pressure_vol]
    type = NekVolumeAverage
    field = pressure
  []
  [avg_temperature_vol]
    type = NekVolumeAverage
    field = temperature
  []

  # heat flux integral
  [nek_flux]
    type = NekHeatFluxIntegral
    boundary = '1'
  []

  # mass flux weighted integral
  [inlet_mdot]
    type = NekMassFluxWeightedSideIntegral
    field = unity
    boundary = '3'
    execute_on = initial
  []
  [outlet_T]
    type = NekMassFluxWeightedSideIntegral
    field = temperature
    boundary = '4'
  []
  [inlet_P]
    type = NekMassFluxWeightedSideIntegral
    field = pressure
    boundary = '4'
  []

  # mass flux weighted integral
  [inlet_mdot_avg]
    type = NekMassFluxWeightedSideAverage
    field = unity
    boundary = '3'
    execute_on = initial
  []
  [outlet_T_avg]
    type = NekMassFluxWeightedSideAverage
    field = temperature
    boundary = '4'
  []
  [inlet_P_avg]
    type = NekMassFluxWeightedSideAverage
    field = pressure
    boundary = '4'
  []

  # extreme value postprocessors - VOLUME
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
  []
  [max_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = max
  []
  [min_p]
    type = NekVolumeExtremeValue
    field = pressure
    value_type = min
  []
  [max_1]
    type = NekVolumeExtremeValue
    field = unity
    value_type = max
  []
  [min_1]
    type = NekVolumeExtremeValue
    field = unity
    value_type = min
  []

  # extreme value postprocessors - SIDE
  [max_T_out]
    type = NekSideExtremeValue
    field = temperature
    boundary = '4'
    value_type = max
  []
  [min_T_out]
    type = NekSideExtremeValue
    field = temperature
    boundary = '4'
    value_type = min
  []
  [max_p_in]
    type = NekSideExtremeValue
    field = pressure
    boundary = '3'
    value_type = max
  []
  [min_p_in]
    type = NekSideExtremeValue
    field = pressure
    boundary = '3'
    value_type = min
  []
  [max_1_in]
    type = NekSideExtremeValue
    field = unity
    boundary = '3'
    value_type = max
  []
  [min_1_in]
    type = NekSideExtremeValue
    field = unity
    boundary = '3'
    value_type = min
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'

  [screen]
    type = Console
    hide = 'synchronization_in'
  []
[]
