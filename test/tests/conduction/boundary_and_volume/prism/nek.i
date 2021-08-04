[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  volume = true
  boundary = '2'
  order = FIRST
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [avg_T_volume]
    type = NekVolumeAverage
    field = temperature
  []
  [heat_flux]
    type = NekHeatFluxIntegral
    boundary = '2'
  []
  [nek_min_1]
    type = NekSideExtremeValue
    field = temperature
    boundary = '3'
    value_type = min
  []
  [nek_min_2]
    type = NekSideExtremeValue
    field = temperature
    boundary = '4'
    value_type = min
  []
  [nek_min_3]
    type = NekSideExtremeValue
    field = temperature
    boundary = '5'
    value_type = min
  []
  [nek_min_4]
    type = NekSideExtremeValue
    field = temperature
    boundary = '6'
    value_type = min
  []
  [nek_max_1]
    type = NekSideExtremeValue
    field = temperature
    boundary = '3'
  []
  [nek_max_2]
    type = NekSideExtremeValue
    field = temperature
    boundary = '4'
  []
  [nek_max_3]
    type = NekSideExtremeValue
    field = temperature
    boundary = '5'
  []
  [nek_max_4]
    type = NekSideExtremeValue
    field = temperature
    boundary = '6'
  []
[]

[Outputs]
  exodus = true
  interval = 30

  hide = 'flux_integral source_integral'
[]
