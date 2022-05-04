[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'turbPipe'

  # We omit the non-dimensional settings here in order to just extract the
  # non-dimensional solution as-is, without dimensionalizing it.
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]
