[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  volume = true
  order = SECOND
  boundary = '2'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [source_integral]
    type = Receiver
  []
  [flux_integral]
    type = Receiver
  []
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
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
