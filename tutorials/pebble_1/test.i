# This file is solely for testing this tutorial, and is not needed to run NekRS
# as a standalone application.

[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[Problem]
  type = NekRSProblem
  casename = 'pebble'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_V]
    type = NekVolumeExtremeValue
    field = velocity_z
  []
[]

[Outputs]
  csv = true
[]
