[Problem]
  type = NekRSProblem
[]

[Mesh]
  type = NekRSMesh
  volume = true
  fixed_mesh = true
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
  [temp_average]
    type = NekVolumeAverage
    field = temperature
  []
  [pressure_average]
    type = NekVolumeAverage
    field = pressure
  []
  [source_integral]
    type = Receiver
  []
[]
