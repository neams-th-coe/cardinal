[Mesh]
  type = NekRSMesh
  boundary = '1'
  volume = true
[]

[Problem]
  type = NekRSProblem
  n_usrwrk_slots = 7
  casename = 'ethier'
[]

[Executioner]
  type = Transient

  [TimeStepper]
    type = NekTimeStepper
  []
[]

[UserObjects]
  [scalar1]
    type = NekScalarValue
    value = 1
    usrwrk_slot = 2
  []
  [scalar2]
    type = NekScalarValue
    value = 1
    usrwrk_slot = 4
  []
  [scalar3]
    type = NekScalarValue
    value = 1
    usrwrk_slot = 5
  []
[]
