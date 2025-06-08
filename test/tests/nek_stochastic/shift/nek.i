[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSProblem
  casename = 'brick'
  n_usrwrk_slots = 3

  [ScalarTransfers]
    [s2]
      type = NekScalarValue
      direction = to_nek
      value = 2.5
      usrwrk_slot = 1
      output_postprocessor = s2
    []
    [s3]
      type = NekScalarValue
      direction = to_nek
      value = 3.5
      usrwrk_slot = 1
      output_postprocessor = s3
    []
  []

  [FieldTransfers]
    [scalar01]
      type = NekFieldVariable
      direction = from_nek
    []
    [scalar02]
      type = NekFieldVariable
      direction = from_nek
    []
    [scalar03]
      type = NekFieldVariable
      direction = from_nek
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
  [s2]
    type = Receiver
  []
  [s3]
    type = Receiver
  []

  [max_scalar1]
    type = NodalExtremeValue
    variable = scalar01
    value_type = max
  []
  [min_scalar1]
    type = NodalExtremeValue
    variable = scalar01
    value_type = min
  []
  [max_scalar2]
    type = NodalExtremeValue
    variable = scalar02
    value_type = max
  []
  [min_scalar2]
    type = NodalExtremeValue
    variable = scalar02
    value_type = min
  []
  [max_scalar3]
    type = NodalExtremeValue
    variable = scalar03
    value_type = max
  []
  [min_scalar3]
    type = NodalExtremeValue
    variable = scalar03
    value_type = min
  []
[]

[Outputs]
  csv = true
[]
