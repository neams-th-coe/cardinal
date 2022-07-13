[Problem]
  type = NekRSProblem
  casename = 'cube'
[]

[Mesh]
  type = NekRSMesh
  volume = true
  parallel_type = replicated
  displacements = 'disp_x disp_y disp_z'
  moving_mesh = true
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
