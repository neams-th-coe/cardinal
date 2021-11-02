[Mesh]
  type = NekRSMesh
  volume = true
  order = SECOND
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'ethier'
  output = 'temperature'

  # We omit the non-dimensional settings here in order to just extract the
  # non-dimensional solution as-is, without dimensionalizing it. You may optionally
  # choose to dimensionalize the solution, depending on what you intend to do with it.
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
[]
