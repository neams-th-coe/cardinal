[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  [out]
    type = CSV
    hide = 'flux_integral source_integral'
    execute_on = 'final'
  []
[]

[Postprocessors]
  [volume]
    type = NekVolumeIntegral
    field = unity
  []
  [temp_integral]
    type = NekVolumeIntegral
    field = temperature
  []
  [flux_integral]
    type = Receiver
  []
  [source_integral]
    type = Receiver
  []
[]
