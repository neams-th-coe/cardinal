[Problem]
  type = NekRSStandaloneProblem
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
  [out]
    type = CSV
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'initial timestep_begin'
  []
[]
