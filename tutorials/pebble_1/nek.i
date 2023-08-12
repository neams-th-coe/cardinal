[Mesh]
  type = NekRSMesh

  # This is the boundary we are coupling via conjugate heat transfer to MOOSE
  boundary = '3'
[]

[Problem]
  type = NekRSProblem
  casename = 'pebble_cht'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
  hide = 'flux_integral'
[]
