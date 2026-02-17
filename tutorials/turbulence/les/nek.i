interval = 500

[Mesh]
  type = NekRSMesh
  boundary = '1'
[]

[Problem]
  type = NekRSProblem
  casename = pipe
  constant_interval = ${interval}
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_yplus]
    type = NekYPlus
    boundary = '1'
    mesh = 'fluid'
  []
  [max_Vx]
    type = NekVolumeExtremeValue
    field = velocity_x
  []
[]

[Outputs]
  time_step_interval = 500
[]
