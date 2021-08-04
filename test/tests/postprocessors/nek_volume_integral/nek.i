[Problem]
  type = NekRSProblem
  casename = 'pyramid'
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
    hide = 'source_integral'
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
  [pressure_integral]
    type = NekVolumeIntegral
    field = pressure
  []
  [velocity_integral]
    type = NekVolumeIntegral
    field = velocity
  []
[]
