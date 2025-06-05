[Mesh]
  type = NekRSMesh
  volume = true
[]

[Problem]
  type = NekRSStandaloneProblem
  casename = 'brick'
  n_usrwrk_slots = 3

  first_reserved_usrwrk_slot = 1

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

[UserObjects]
  [scalar2]
    type = NekScalarValue
    value = 2.5
  []
  [scalar3]
    type = NekScalarValue
    value = 3.5
  []
[]

[Postprocessors]
  [s2]
    type = NekScalarValuePostprocessor
    userobject = scalar2
  []
  [s3]
    type = NekScalarValuePostprocessor
    userobject = scalar3
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
