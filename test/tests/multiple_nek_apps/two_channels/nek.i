[Mesh]
  type = NekRSMesh
  boundary = '1'
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'pin'

  # this just helps us skip the heat source transfer into NekRS from MOOSE
  # (infrastructure that exists for coupling to neutronics) when we could
  # easily just set the constant pin heat source ourselves
  has_heat_source = false
[]

[Postprocessors]
  [max_T]
    type = NekVolumeExtremeValue
    field = temperature
  []
  [avg_T]
    type = NekVolumeAverage
    field = temperature
  []
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'final'

  # just to make the gold files smaller
  hide = 'avg_flux flux_integral'
[]
