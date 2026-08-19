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

[Functions]
  [unit_function]
    type = ParsedFunction
    expression = '1'
  []
[]

[Postprocessors]
  [invalid]
    type = NekVolumeIntegral
  []
[]
