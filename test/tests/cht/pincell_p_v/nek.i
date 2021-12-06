[Problem]
  type = NekRSProblem
  casename = 'sfr_pin'
  output = 'pressure velocity'
  has_heat_source = false
[]

[Mesh]
  type = NekRSMesh
  order = SECOND
  boundary = '1'

  # we dont have any volume-based coupling with NekRS, but by specifying
  # volume here, we will extract pressure and velocity on
  # a volume mesh mirror
  volume = true
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
  execute_on = 'final'
[]
