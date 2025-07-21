[Problem]
  type = NekRSProblem
  casename = 'sfr_pin'
  n_usrwrk_slots = 1

  [FieldTransfers]
    [avg_flux]
      type = NekBoundaryFlux
      direction = to_nek
      usrwrk_slot = 0
      postprocessor_to_conserve = flux_integral
      conserve_flux_by_sideset = true
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
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [nek_flux]
    type = NekHeatFluxIntegral
    boundary = '1'
  []
  [average_inlet_T]
    type = NekSideAverage
    field = temperature
    boundary = '3'
    execute_on = initial
  []
  [average_outlet_T]
    type = NekSideAverage
    field = temperature
    boundary = '4'
  []
  [dT]
    type = DifferencePostprocessor
    value1 = average_outlet_T
    value2 = average_inlet_T
  []
  [inlet_mdot]
    type = NekMassFluxWeightedSideIntegral
    field = unity
    boundary = '3'
    execute_on = initial
  []

  # postprocessors for comparing against non-dimensional version in ../nondimensional
  # --> uncomment in order to get the reference values that the nondimensional boundary coupling
  #     was verified against
  #
  # # side integral
  # [area_1]
  #   type = NekSideIntegral
  #   field = unity
  #   boundary = '1'
  # []
  # [pressure_1]
  #   type = NekSideIntegral
  #   field = pressure
  #   boundary = '1'
  # []
  # [temperature_1]
  #   type = NekSideIntegral
  #   field = temperature
  #   boundary = '1'
  # []

  # # side average
  # [avg_area_1]
  #   type = NekSideAverage
  #   field = unity
  #   boundary = '1'
  # []
  # [avg_pressure_1]
  #   type = NekSideAverage
  #   field = pressure
  #   boundary = '1'
  # []
  # [avg_temperature_1]
  #   type = NekSideAverage
  #   field = temperature
  #   boundary = '1'
  # []

  # # volume integral
  # [volume]
  #   type = NekVolumeIntegral
  #   field = unity
  # []
  # [pressure_vol]
  #   type = NekVolumeIntegral
  #   field = pressure
  # []
  # [temperature_vol]
  #   type = NekVolumeIntegral
  #   field = temperature
  # []

  # # volume average
  # [avg_volume]
  #   type = NekVolumeAverage
  #   field = unity
  # []
  # [avg_pressure_vol]
  #   type = NekVolumeAverage
  #   field = pressure
  # []
  # [avg_temperature_vol]
  #   type = NekVolumeAverage
  #   field = temperature
  # []

  # # heat flux integral
  # [nek_flux]
  #   type = NekHeatFluxIntegral
  #   boundary = '1'
  # []

  # # mass flux weighted integral
  # [inlet_mdot]
  #   type = NekMassFluxWeightedSideIntegral
  #   field = unity
  #   boundary = '3'
  #   execute_on = initial
  # []
  # [outlet_T]
  #   type = NekMassFluxWeightedSideIntegral
  #   field = temperature
  #   boundary = '4'
  # []
  # [inlet_P]
  #   type = NekMassFluxWeightedSideIntegral
  #   field = pressure
  #   boundary = '4'
  # []

  # # mass flux weighted integral
  # [inlet_mdot_avg]
  #   type = NekMassFluxWeightedSideAverage
  #   field = unity
  #   boundary = '3'
  #   execute_on = initial
  # []
  # [outlet_T_avg]
  #   type = NekMassFluxWeightedSideAverage
  #   field = temperature
  #   boundary = '4'
  # []
  # [inlet_P_avg]
  #   type = NekMassFluxWeightedSideAverage
  #   field = pressure
  #   boundary = '4'
  # []

  # # extreme value postprocessors - VOLUME
  # [max_T]
  #   type = NekVolumeExtremeValue
  #   field = temperature
  #   value_type = max
  # []
  # [min_T]
  #   type = NekVolumeExtremeValue
  #   field = temperature
  #   value_type = min
  # []
  # [max_p]
  #   type = NekVolumeExtremeValue
  #   field = pressure
  #   value_type = max
  # []
  # [min_p]
  #   type = NekVolumeExtremeValue
  #   field = pressure
  #   value_type = min
  # []
  # [max_1]
  #   type = NekVolumeExtremeValue
  #   field = unity
  #   value_type = max
  # []
  # [min_1]
  #   type = NekVolumeExtremeValue
  #   field = unity
  #   value_type = min
  # []

  # # extreme value postprocessors - SIDE
  # [max_T_out]
  #   type = NekSideExtremeValue
  #   field = temperature
  #   boundary = '4'
  #   value_type = max
  # []
  # [min_T_out]
  #   type = NekSideExtremeValue
  #   field = temperature
  #   boundary = '4'
  #   value_type = min
  # []
  # [max_p_in]
  #   type = NekSideExtremeValue
  #   field = pressure
  #   boundary = '3'
  #   value_type = max
  # []
  # [min_p_in]
  #   type = NekSideExtremeValue
  #   field = pressure
  #   boundary = '3'
  #   value_type = min
  # []
  # [max_1_in]
  #   type = NekSideExtremeValue
  #   field = unity
  #   boundary = '3'
  #   value_type = max
  # []
  # [min_1_in]
  #   type = NekSideExtremeValue
  #   field = unity
  #   boundary = '3'
  #   value_type = min
  # []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
  csv = true

  [screen]
    type = Console
    hide = 'average_inlet_T average_outlet_T'
  []
[]
