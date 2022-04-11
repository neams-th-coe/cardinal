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

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_S]
    type = NekVolumeExtremeValue
  []
[]
