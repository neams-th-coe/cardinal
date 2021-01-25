[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  boundary = '2'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [flux_integral]
    type = Receiver
    default = 0
  []
  [max_temp_nek]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = max
  []
  [min_temp_nek]
    type = NekVolumeExtremeValue
    field = temperature
    value_type = min
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
