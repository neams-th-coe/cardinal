[Mesh]
  type = NekRSMesh
  volume = true
  exact = true
[]

[Problem]
  type = NekRSProblem
  casename = 'read1'

  [FieldTransfers]
    [scalar01]
      type = NekFieldVariable
      direction = from_nek
      field = scalar01
    []
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = NekTimeStepper
  []
[]

[Postprocessors]
  [max_scalar]
    type = NodalExtremeValue
    variable = scalar01
    value_type = max
  []
  [min_scalar]
    type = NodalExtremeValue
    variable = scalar01
    value_type = min
  []
  [max_scalar_nek]
    type = NekVolumeExtremeValue
    field = scalar01
    value_type = max
  []
  [min_scalar_nek]
    type = NekVolumeExtremeValue
    field = scalar01
    value_type = min
  []
[]

[Outputs]
  csv = true
[]
