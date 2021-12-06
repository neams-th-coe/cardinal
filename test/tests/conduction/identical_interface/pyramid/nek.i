[Problem]
  type = NekRSProblem
  casename = 'pyramid'
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
