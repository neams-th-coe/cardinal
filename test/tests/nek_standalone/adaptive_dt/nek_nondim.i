[Problem]
  type = NekRSProblem
  casename = 'pyramid'

  [Dimensionalize]
    U = 0.1
  []
[]

[Mesh]
  type = NekRSMesh
  scaling = 1.0
  volume = true
[]

[Executioner]
  type = Transient

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
