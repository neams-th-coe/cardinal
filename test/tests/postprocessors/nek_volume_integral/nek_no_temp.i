[Problem]
  type = NekRSProblem
  casename = 'pyramid_no_temp'
[]

[Mesh]
  type = NekRSMesh
  volume = true
  exact = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

[Postprocessors]
  [s01_int]
    type = NekVolumeIntegral
    field = scalar01
  []
  [s02_int]
    type = NekVolumeIntegral
    field = scalar02
  []
  [s03_int]
    type = NekVolumeIntegral
    field = scalar03
  []
[]
