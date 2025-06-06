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
  start_time = 1.0

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'initial timestep_begin'
  []
[]
