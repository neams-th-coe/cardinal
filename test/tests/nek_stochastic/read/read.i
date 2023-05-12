[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'read'
  output = 'scalar02'
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_scalar]
    type = ElementExtremeValue
    variable = scalar02
    value_type = max
  []
  [min_scalar]
    type = ElementExtremeValue
    variable = scalar02
    value_type = min
  []
[]

[Outputs]
  csv = true
[]
