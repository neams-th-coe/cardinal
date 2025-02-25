[Problem]
  type = NekRSStandaloneProblem
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
